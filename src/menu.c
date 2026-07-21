/*
 * declarative_menu.c
 * Production-grade, dependency-free C TUI menu with dynamic allocation.
 * Background: RGB(255, 250, 240) warm ivory.
 *
 * Compile: gcc -O2 -o menu declarative_menu.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <locale.h>

#define ESC          "\033["
#define ALT_BUF_ON   ESC "?1049h"
#define ALT_BUF_OFF  ESC "?1049l"
#define CLEAR_SCREEN ESC "H" ESC "2J" ESC "3J"
#define MOUSE_ON     ESC "?1000h" ESC "?1002h" ESC "?1015h" ESC "?1006h"
#define MOUSE_OFF    ESC "?1006l" ESC "?1015l" ESC "?1002l" ESC "?1000l"
#define RESET_COLOR  ESC "0m"

/* ANSI helpers */
static void fgRgb(int r, int g, int b) {
    printf(ESC "38;2;%d;%d;%dm", r, g, b);
}
static void bgRgb(int r, int g, int b) {
    printf(ESC "48;2;%d;%d;%dm", r, g, b);
}
static void moveCursor(int row, int col) {
    printf(ESC "%d;%dH", row, col);
}

/* RGB color */
typedef struct {
    unsigned char r, g, b;
} Color;

#define C(r,g,b) ((Color){r,g,b})

/* Default palette */
static const Color cBg       = {255, 250, 240}; /* warm ivory */
static const Color cFg       = {45, 55, 72};    /* dark slate */
static const Color cBorder   = {160, 174, 192}; /* soft steel */
static const Color cShadow   = {200, 190, 180}; /* warm grey */
static const Color cTabFg    = {113, 128, 150}; /* muted blue-grey */
static const Color cTabAct   = {59, 130, 246};  /* bright blue */
static const Color cHlBg     = {59, 130, 246};  /* highlight blue */
static const Color cHlFg     = {255, 255, 255};  /* white */

/* Color enum for menuColor() */
enum {
    MENU_COLOR_BG,
    MENU_COLOR_FG,
    MENU_COLOR_BORDER,
    MENU_COLOR_SHADOW,
    MENU_COLOR_TAB_FG,
    MENU_COLOR_TAB_ACTIVE,
    MENU_COLOR_HL_BG,
    MENU_COLOR_HL_FG
};

/* Background draw callback: user provides a function to paint the whole screen */
typedef void (*MenuBgDrawFn)(int termW, int termH, void *userData);

/* Single menu item */
typedef struct {
    char *label;
    int icon; /* UTF-32 codepoint, 0 = none */
} MenuItem;

/* Single tab containing items */
typedef struct {
    char *title;
    MenuItem *items;
    int itemCount;
    int itemCap;
    int initialSel;
} MenuTab;

/* Menu configuration and state */
typedef struct Menu {
    MenuTab *tabs;
    int tabCount;
    int tabCap;
    int initialTab;
    int x, y, w, h; /* position and size, 0 = auto-center/auto-size */
    Color bg, fg, border, shadow, tabFg, tabActive, hlBg, hlFg;
    int hasBg, hasFg, hasBorder, hasShadow;
    int hasTabFg, hasTabActive, hasHlBg, hasHlFg;
    int shadowEnabled;
    int borderDouble;
    int roundedCorners;
    int useAltBuffer;  /* 1 = use alternate screen buffer */
    int useMouse;      /* 1 = enable mouse tracking */
    MenuBgDrawFn bgDrawFn;
    void *bgDrawData;
} Menu;

/* Result returned by menuRun() */
typedef struct {
    int selectedTab;
    int selectedItem;
} MenuResult;

/* Internal runtime state */
typedef struct {
    int termW, termH;
    int menuX, menuY;
    int menuW, menuH;
    int activeTab;
    int selIdx;
    int running;
    int confirmed;
    struct termios origTerm;
} MenuState;

static MenuState gState;
static volatile sig_atomic_t gResized = 0;

static void onResize(int sig) {
    (void)sig;
    gResized = 1;
}

static void termRaw(void) {
    struct termios t;
    tcgetattr(STDIN_FILENO, &gState.origTerm);
    t = gState.origTerm;
    t.c_lflag &= ~(ECHO | ICANON | ISIG);
    t.c_iflag &= ~(IXON | ICRNL | INLCR);
    t.c_cc[VMIN]  = 0;
    t.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

static void termRestore(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &gState.origTerm);
}

static void getTermSize(void) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
        gState.termW = ws.ws_col;
        gState.termH = ws.ws_row;
    } else {
        gState.termW = 80;
        gState.termH = 24;
    }
}

static void putUtf8(int codepoint) {
    if (codepoint <= 0x7F) {
        putchar(codepoint);
    } else if (codepoint <= 0x7FF) {
        putchar(0xC0 | (codepoint >> 6));
        putchar(0x80 | (codepoint & 0x3F));
    } else if (codepoint <= 0xFFFF) {
        putchar(0xE0 | (codepoint >> 12));
        putchar(0x80 | ((codepoint >> 6) & 0x3F));
        putchar(0x80 | (codepoint & 0x3F));
    } else {
        putchar(0xF0 | (codepoint >> 18));
        putchar(0x80 | ((codepoint >> 12) & 0x3F));
        putchar(0x80 | ((codepoint >> 6) & 0x3F));
        putchar(0x80 | (codepoint & 0x3F));
    }
}

/* Display width of a UTF-8 string (ASCII=1, CJK/wide=2) */
static int displayWidth(const char *s) {
    int w = 0;
    const unsigned char *p = (const unsigned char *)s;
    while (*p) {
        if (*p < 0x80) {
            w++;
            p++;
        } else if ((*p & 0xE0) == 0xC0) {
            w += 2; p += 2;
        } else if ((*p & 0xF0) == 0xE0) {
            w += 2; p += 3;
        } else if ((*p & 0xF8) == 0xF0) {
            w += 2; p += 4;
        } else {
            p++;
        }
    }
    return w;
}

static void drawCharAt(int row, int col, int codepoint, const Color *bg, const Color *fg) {
    moveCursor(row, col);
    if (bg) bgRgb(bg->r, bg->g, bg->b);
    if (fg) fgRgb(fg->r, fg->g, fg->b);
    putUtf8(codepoint);
}

static void drawStrAt(int row, int col, const char *s, const Color *bg, const Color *fg) {
    moveCursor(row, col);
    if (bg) bgRgb(bg->r, bg->g, bg->b);
    if (fg) fgRgb(fg->r, fg->g, fg->b);
    printf("%s", s);
}

static void fillRect(int r, int c, int h, int w, const Color *bg) {
    int i;
    char buf[512];
    int len = 0;
    for (i = 0; i < w && i < 255; i++) buf[len++] = ' ';
    buf[len] = '\0';
    for (i = 0; i < h; i++) {
        moveCursor(r + i, c);
        if (bg) bgRgb(bg->r, bg->g, bg->b);
        printf("%s", buf);
    }
}

static void drawShadow(int r, int c, int h, int w, const Color *shadow) {
    int i;
    const Color *s = shadow ? shadow : &cShadow;
    Color dim;
    dim.r = (s->r + cBg.r) / 2;
    dim.g = (s->g + cBg.g) / 2;
    dim.b = (s->b + cBg.b) / 2;
    for (i = 1; i < h; i++) {
        drawCharAt(r + i, c + w, 0x2591, &dim, NULL); /* light shade */
    }
    for (i = 0; i < w; i++) {
        drawCharAt(r + h, c + i + 1, 0x2591, &dim, NULL);
    }
    drawCharAt(r + h, c + w, 0x2591, &dim, NULL);
}

static void drawBorder(int r, int c, int h, int w,
                        const Color *border, int doubleLine, int rounded) {
    int i;
    const Color *b = border ? border : &cBorder;
    int tl, tr, bl, br, hz, vt;

    if (doubleLine) {
        tl = rounded ? 0x256D : 0x2554;
        tr = rounded ? 0x256E : 0x2557;
        bl = rounded ? 0x2570 : 0x255A;
        br = rounded ? 0x256F : 0x255D;
        hz = 0x2550;
        vt = 0x2551;
    } else {
        tl = rounded ? 0x256D : 0x250C;
        tr = rounded ? 0x256E : 0x2510;
        bl = rounded ? 0x2570 : 0x2514;
        br = rounded ? 0x256F : 0x2518;
        hz = 0x2500;
        vt = 0x2502;
    }

    fgRgb(b->r, b->g, b->b);

    moveCursor(r, c);
    putUtf8(tl);
    for (i = 0; i < w - 2; i++) putUtf8(hz);
    putUtf8(tr);

    for (i = 1; i < h - 1; i++) {
        drawCharAt(r + i, c, vt, NULL, b);
        drawCharAt(r + i, c + w - 1, vt, NULL, b);
    }

    moveCursor(r + h - 1, c);
    putUtf8(bl);
    for (i = 0; i < w - 2; i++) putUtf8(hz);
    putUtf8(br);

    printf(RESET_COLOR);
}

static void drawTabBar(int r, int c, int w, const Menu *m) {
    int i;
    const Color *bg  = m->hasBg ? &m->bg : &cBg;
    const Color *fg  = m->hasTabFg ? &m->tabFg : &cTabFg;
    const Color *act = m->hasTabActive ? &m->tabActive : &cTabAct;
    const Color *bdr = m->hasBorder ? &m->border : &cBorder;

    fillRect(r, c + 1, 1, w - 2, bg);

    int tabX = c + 2;
    for (i = 0; i < m->tabCount; i++) {
        int len = displayWidth(m->tabs[i].title);
        int isActive = (i == gState.activeTab);
        const Color *txt = isActive ? act : fg;
        int tabTotalW = len + 4; /* [ text ] */

        if (tabX + tabTotalW >= c + w - 1) break;

        if (isActive) {
            drawCharAt(r, tabX, '[', NULL, act);
            drawCharAt(r, tabX + 1, ' ', bg, NULL);
            drawStrAt(r, tabX + 2, m->tabs[i].title, bg, txt);
            drawCharAt(r, tabX + 2 + len, ' ', bg, NULL);
            drawCharAt(r, tabX + 3 + len, ']', NULL, act);
        } else {
            drawCharAt(r, tabX, ' ', bg, NULL);
            drawCharAt(r, tabX + 1, ' ', bg, NULL);
            drawStrAt(r, tabX + 2, m->tabs[i].title, bg, txt);
            drawCharAt(r, tabX + 2 + len, ' ', bg, NULL);
            drawCharAt(r, tabX + 3 + len, ' ', bg, NULL);
        }
        tabX += tabTotalW + 1;
    }

    /* separator line between tab bar and item list */
    moveCursor(r + 1, c + 1);
    bgRgb(bg->r, bg->g, bg->b);
    fgRgb(bdr->r, bdr->g, bdr->b);
    for (i = 0; i < w - 2; i++) {
        putUtf8(0x2500);
    }
    printf(RESET_COLOR);
}

static void drawItems(int r, int c, int h, int w,
                       const Menu *m, const MenuTab *tab) {
    int i;
    int contentH = h - 4;
    int startY = r + 3;
    const Color *bg  = m->hasBg ? &m->bg : &cBg;
    const Color *fgc = m->hasFg ? &m->fg : &cFg;
    const Color *hlb = m->hasHlBg ? &m->hlBg : &cHlBg;
    const Color *hlf = m->hasHlFg ? &m->hlFg : &cHlFg;

    for (i = 0; i < contentH && i < tab->itemCount; i++) {
        int isSel = (i == gState.selIdx);
        const Color *itemBg = isSel ? hlb : bg;
        const Color *itemFg = isSel ? hlf : fgc;
        int y = startY + i;
        int labelW = displayWidth(tab->items[i].label);
        int iconW = tab->items[i].icon ? 2 : 0;
        int totalW = labelW + iconW;
        int padLeft = 2;
        int padRight = w - 2 - totalW - padLeft;
        if (padRight < 0) padRight = 0;

        moveCursor(y, c + 1);
        bgRgb(itemBg->r, itemBg->g, itemBg->b);
        fgRgb(itemFg->r, itemFg->g, itemFg->b);

        int p;
        for (p = 0; p < padLeft; p++) putchar(' ');

        if (tab->items[i].icon) {
            putUtf8(tab->items[i].icon);
            putchar(' ');
        }

        printf("%s", tab->items[i].label);

        for (p = 0; p < padRight; p++) putchar(' ');

        printf(RESET_COLOR);
    }

    for (i = tab->itemCount; i < contentH; i++) {
        moveCursor(startY + i, c + 1);
        bgRgb(bg->r, bg->g, bg->b);
        int p;
        for (p = 0; p < w - 2; p++) putchar(' ');
        printf(RESET_COLOR);
    }
}

/* Full redraw: background + border + tab bar + items. Used on init and resize. */
static void drawMenuFull(const Menu *m) {
    int r = gState.menuY;
    int c = gState.menuX;
    int w = gState.menuW;
    int h = gState.menuH;
    const Color *bg = m->hasBg ? &m->bg : &cBg;

    /* User background callback: paint the whole screen first */
    if (m->bgDrawFn) {
        m->bgDrawFn(gState.termW, gState.termH, m->bgDrawData);
    } else {
        fillRect(r, c, h, w, bg);
    }

    if (m->shadowEnabled) {
        const Color *sh = m->hasShadow ? &m->shadow : &cShadow;
        drawShadow(r, c, h, w, sh);
    }

    drawBorder(r, c, h, w, m->hasBorder ? &m->border : NULL,
                m->borderDouble, m->roundedCorners);
    drawTabBar(r + 1, c, w, m);
    drawItems(r, c, h, w, m, &m->tabs[gState.activeTab]);

    fflush(stdout);
}

/* Partial redraw: only tab bar + items. Used on tab switch. Background unchanged. */
static void drawMenuContent(const Menu *m) {
    int r = gState.menuY;
    int c = gState.menuX;
    int w = gState.menuW;
    int h = gState.menuH;

    drawTabBar(r + 1, c, w, m);
    drawItems(r, c, h, w, m, &m->tabs[gState.activeTab]);

    fflush(stdout);
}

/* Redraw only a single item row (for efficient updates) */
static void drawItemAt(const Menu *m, int idx) {
    int r = gState.menuY;
    int c = gState.menuX;
    int w = gState.menuW;
    int h = gState.menuH;
    int contentH = h - 4;
    int startY = r + 3;
    const MenuTab *tab = &m->tabs[gState.activeTab];
    const Color *bg  = m->hasBg ? &m->bg : &cBg;
    const Color *fgc = m->hasFg ? &m->fg : &cFg;
    const Color *hlb = m->hasHlBg ? &m->hlBg : &cHlBg;
    const Color *hlf = m->hasHlFg ? &m->hlFg : &cHlFg;

    if (idx < 0 || idx >= contentH || idx >= tab->itemCount) return;

    int isSel = (idx == gState.selIdx);
    const Color *itemBg = isSel ? hlb : bg;
    const Color *itemFg = isSel ? hlf : fgc;
    int y = startY + idx;
    int labelW = displayWidth(tab->items[idx].label);
    int iconW = tab->items[idx].icon ? 2 : 0;
    int totalW = labelW + iconW;
    int padLeft = 2;
    int padRight = w - 2 - totalW - padLeft;
    if (padRight < 0) padRight = 0;

    moveCursor(y, c + 1);
    bgRgb(itemBg->r, itemBg->g, itemBg->b);
    fgRgb(itemFg->r, itemFg->g, itemFg->b);

    int p;
    for (p = 0; p < padLeft; p++) putchar(' ');

    if (tab->items[idx].icon) {
        putUtf8(tab->items[idx].icon);
        putchar(' ');
    }

    printf("%s", tab->items[idx].label);

    for (p = 0; p < padRight; p++) putchar(' ');

    printf(RESET_COLOR);
    fflush(stdout);
}

/* Input event types */
#define KEY_UP    1
#define KEY_DOWN  2
#define KEY_LEFT  3
#define KEY_RIGHT 4
#define KEY_ENTER 5
#define KEY_ESC   6
#define KEY_MOUSE 7
#define KEY_NONE  0

typedef struct {
    int type;
    int mouseX;
    int mouseY;
    int mouseBtn;
    int mousePress; /* 1=press (M), 0=release (m) */
} InputEvent;

static int readByte(void) {
    unsigned char ch;
    int n = read(STDIN_FILENO, &ch, 1);
    return (n == 1) ? ch : -1;
}

static InputEvent parseInput(void) {
    InputEvent ev = {KEY_NONE, 0, 0, 0, 0};
    int c = readByte();
    if (c < 0) return ev;

    if (c == 27) {
        int c2 = readByte();
        if (c2 < 0) { ev.type = KEY_ESC; return ev; }

        if (c2 == '[') {
            int c3 = readByte();
            if (c3 < 0) return ev;

            if (c3 == 'A') { ev.type = KEY_UP; return ev; }
            if (c3 == 'B') { ev.type = KEY_DOWN; return ev; }
            if (c3 == 'C') { ev.type = KEY_RIGHT; return ev; }
            if (c3 == 'D') { ev.type = KEY_LEFT; return ev; }

            /* SGR mouse protocol: ESC [ < btn ; x ; y (M|m) */
            if (c3 == '<') {
                int btn = 0, mx = 0, my = 0, n;
                char buf[32];
                int i = 0;
                int lastCh = 0;
                while (i < 31) {
                    int ch = readByte();
                    if (ch < 0) break;
                    buf[i++] = (char)ch;
                    if (ch == 'M' || ch == 'm') {
                        lastCh = ch;
                        break;
                    }
                }
                buf[i] = '\0';
                n = sscanf(buf, "%d;%d;%d", &btn, &mx, &my);
                if (n >= 3) {
                    ev.type = KEY_MOUSE;
                    ev.mouseX = mx;
                    ev.mouseY = my;
                    ev.mouseBtn = btn;
                    ev.mousePress = (lastCh == 'M') ? 1 : 0;
                }
                return ev;
            }
        }
    }

    if (c == 'w' || c == 'W' || c == 'k' || c == 'K') { ev.type = KEY_UP; return ev; }
    if (c == 's' || c == 'S' || c == 'j' || c == 'J') { ev.type = KEY_DOWN; return ev; }
    if (c == 'a' || c == 'A' || c == 'h' || c == 'H') { ev.type = KEY_LEFT; return ev; }
    if (c == 'd' || c == 'D' || c == 'l' || c == 'L') { ev.type = KEY_RIGHT; return ev; }

    if (c == 10 || c == 13) { ev.type = KEY_ENTER; return ev; }

    if (c == 3 || c == 4 || c == 'q' || c == 'Q') { ev.type = KEY_ESC; return ev; }

    return ev;
}

static int hitTestItem(int mx, int my, const Menu *m) {
    int r = gState.menuY;
    int c = gState.menuX;
    int w = gState.menuW;
    int h = gState.menuH;
    int contentStartY = r + 3;
    int contentH = h - 4;
    const MenuTab *tab = &m->tabs[gState.activeTab];

    if (mx < c + 1 || mx >= c + w - 1) return -1;
    if (my < contentStartY || my >= contentStartY + contentH) return -1;

    int idx = my - contentStartY;
    if (idx < 0 || idx >= tab->itemCount) return -1;
    return idx;
}

static int hitTestTab(int mx, int my, const Menu *m) {
    int r = gState.menuY;
    int c = gState.menuX;
    int w = gState.menuW;
    int tabY = r + 1;
    int tabXStart = c + 2;
    int i;
    int currentX = tabXStart;

    if (my != tabY) return -1;
    if (mx < tabXStart || mx >= c + w - 1) return -1;

    for (i = 0; i < m->tabCount; i++) {
        int len = displayWidth(m->tabs[i].title);
        int tabTotalW = len + 4;
        if (mx >= currentX && mx < currentX + tabTotalW) {
            return i;
        }
        currentX += tabTotalW + 1;
    }
    return -1;
}

static void menuInitGeometry(const Menu *m) {
    int maxItems = 0;
    int maxLabel = 0;
    int i, j;
    int tabBarWidth = 0;

    getTermSize();

    for (i = 0; i < m->tabCount; i++) {
        tabBarWidth += displayWidth(m->tabs[i].title) + 5;
        if (m->tabs[i].itemCount > maxItems)
            maxItems = m->tabs[i].itemCount;
        for (j = 0; j < m->tabs[i].itemCount; j++) {
            int len = displayWidth(m->tabs[i].items[j].label);
            if (m->tabs[i].items[j].icon) len += 2;
            if (len > maxLabel) maxLabel = len;
        }
    }

    gState.menuW = m->w;
    if (gState.menuW == 0) {
        gState.menuW = maxLabel + 8;
        if (tabBarWidth + 4 > gState.menuW)
            gState.menuW = tabBarWidth + 4;
        if (gState.menuW < 24) gState.menuW = 24;
        if (gState.menuW > gState.termW - 4)
            gState.menuW = gState.termW - 4;
    }

    gState.menuH = m->h;
    if (gState.menuH == 0) {
        gState.menuH = maxItems + 5;
        if (gState.menuH < 8) gState.menuH = 8;
        if (gState.menuH > gState.termH - 2)
            gState.menuH = gState.termH - 2;
    }

    gState.menuX = m->x;
    gState.menuY = m->y;
    if (gState.menuX == 0) gState.menuX = (gState.termW - gState.menuW) / 2;
    if (gState.menuY == 0) gState.menuY = (gState.termH - gState.menuH) / 2;
    if (gState.menuX < 1) gState.menuX = 1;
    if (gState.menuY < 1) gState.menuY = 1;

    gState.activeTab = m->initialTab;
    if (gState.activeTab < 0) gState.activeTab = 0;
    if (gState.activeTab >= m->tabCount) gState.activeTab = 0;
    gState.selIdx = m->tabs[gState.activeTab].initialSel;
    if (gState.selIdx < 0) gState.selIdx = 0;
    if (gState.selIdx >= m->tabs[gState.activeTab].itemCount)
        gState.selIdx = 0;
}

/* ================================================================== */
/* Public API                                                         */
/* ================================================================== */

/* Create a new empty menu with default settings */
Menu* menuCreate(void) {
    Menu *m = calloc(1, sizeof(Menu));
    if (!m) return NULL;
    m->tabCap = 4;
    m->tabs = calloc(m->tabCap, sizeof(MenuTab));
    if (!m->tabs) { free(m); return NULL; }
    m->initialTab = 0;
    m->shadowEnabled = 1;
    m->borderDouble = 0;
    m->roundedCorners = 1;
    m->useAltBuffer = 1;
    m->useMouse = 1;
    return m;
}

/* Free all memory allocated by the menu */
void menuFree(Menu *m) {
    int i, j;
    if (!m) return;
    for (i = 0; i < m->tabCount; i++) {
        free(m->tabs[i].title);
        for (j = 0; j < m->tabs[i].itemCount; j++) {
            free(m->tabs[i].items[j].label);
        }
        free(m->tabs[i].items);
    }
    free(m->tabs);
    free(m);
}

/* Add a new tab, return its index. Auto-resizes. */
int menuTab(Menu *m, const char *title) {
    MenuTab *t;
    if (!m || !title) return -1;
    if (m->tabCount >= m->tabCap) {
        int newCap = m->tabCap * 2;
        MenuTab *newTabs = realloc(m->tabs, newCap * sizeof(MenuTab));
        if (!newTabs) return -1;
        memset(newTabs + m->tabCap, 0, (newCap - m->tabCap) * sizeof(MenuTab));
        m->tabs = newTabs;
        m->tabCap = newCap;
    }
    t = &m->tabs[m->tabCount];
    t->title = strdup(title);
    t->itemCap = 8;
    t->items = calloc(t->itemCap, sizeof(MenuItem));
    t->initialSel = 0;
    return m->tabCount++;
}

/* Add an item to the specified tab. Auto-resizes. */
int menuItem(Menu *m, int tabIdx, const char *label, int icon) {
    MenuTab *t;
    MenuItem *it;
    if (!m || !label || tabIdx < 0 || tabIdx >= m->tabCount) return -1;
    t = &m->tabs[tabIdx];
    if (t->itemCount >= t->itemCap) {
        int newCap = t->itemCap * 2;
        MenuItem *newItems = realloc(t->items, newCap * sizeof(MenuItem));
        if (!newItems) return -1;
        memset(newItems + t->itemCap, 0, (newCap - t->itemCap) * sizeof(MenuItem));
        t->items = newItems;
        t->itemCap = newCap;
    }
    it = &t->items[t->itemCount];
    it->label = strdup(label);
    it->icon = icon;
    return t->itemCount++;
}

/* Set a color by enum: MENU_COLOR_BG, MENU_COLOR_FG, etc. */
void menuColor(Menu *m, int which, Color c) {
    if (!m) return;
    switch (which) {
        case MENU_COLOR_BG:       m->bg = c; m->hasBg = 1; break;
        case MENU_COLOR_FG:       m->fg = c; m->hasFg = 1; break;
        case MENU_COLOR_BORDER:   m->border = c; m->hasBorder = 1; break;
        case MENU_COLOR_SHADOW:   m->shadow = c; m->hasShadow = 1; break;
        case MENU_COLOR_TAB_FG:   m->tabFg = c; m->hasTabFg = 1; break;
        case MENU_COLOR_TAB_ACTIVE: m->tabActive = c; m->hasTabActive = 1; break;
        case MENU_COLOR_HL_BG:    m->hlBg = c; m->hasHlBg = 1; break;
        case MENU_COLOR_HL_FG:    m->hlFg = c; m->hasHlFg = 1; break;
    }
}

/* Set visual style: shadow, double border, rounded corners */
void menuStyle(Menu *m, int shadow, int doubleBorder, int rounded) {
    if (!m) return;
    m->shadowEnabled = shadow;
    m->borderDouble = doubleBorder;
    m->roundedCorners = rounded;
}

/* Set position and size. Use 0 for auto-center / auto-size. */
void menuPos(Menu *m, int x, int y, int w, int h) {
    if (!m) return;
    m->x = x; m->y = y; m->w = w; m->h = h;
}

/* Set initially selected tab and item */
void menuInitial(Menu *m, int tab, int item) {
    if (!m) return;
    m->initialTab = tab;
    if (tab >= 0 && tab < m->tabCount)
        m->tabs[tab].initialSel = item;
}

/* Set whether to use alternate screen buffer (default: 1) */
void menuUseAltBuffer(Menu *m, int use) {
    if (!m) return;
    m->useAltBuffer = use;
}

/* Set whether to enable mouse tracking (default: 1) */
void menuUseMouse(Menu *m, int use) {
    if (!m) return;
    m->useMouse = use;
}

/* Set user background draw callback. Pass NULL to disable. */
void menuBgDraw(Menu *m, MenuBgDrawFn fn, void *userData) {
    if (!m) return;
    m->bgDrawFn = fn;
    m->bgDrawData = userData;
}

/* Run the interactive menu. Returns selected tab/item, or {-1,-1} on cancel. */
MenuResult menuRun(Menu *m) {
    MenuResult res = {-1, -1};
    InputEvent ev;
    const MenuTab *tab;
    struct sigaction sa, oldSa;

    if (!m || m->tabCount == 0) return res;

    memset(&gState, 0, sizeof(gState));
    menuInitGeometry(m);

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = onResize;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGWINCH, &sa, &oldSa);

    if (m->useAltBuffer) printf(ALT_BUF_ON);
    printf(CLEAR_SCREEN);
    if (m->useMouse) printf(MOUSE_ON);
    termRaw();

    gState.running = 1;
    gState.confirmed = 0;

    printf(CLEAR_SCREEN);
    drawMenuFull(m);

    while (gState.running) {
        if (gResized) {
            gResized = 0;
            getTermSize();
            menuInitGeometry(m);
            printf(CLEAR_SCREEN);
            drawMenuFull(m);
        }

        ev = parseInput();
        tab = &m->tabs[gState.activeTab];

        switch (ev.type) {
            case KEY_UP:
                if (gState.selIdx > 0) {
                    int oldIdx = gState.selIdx;
                    gState.selIdx--;
                    drawItemAt(m, oldIdx);
                    drawItemAt(m, gState.selIdx);
                } else {
                    int oldIdx = gState.selIdx;
                    gState.selIdx = tab->itemCount - 1;
                    drawItemAt(m, oldIdx);
                    drawItemAt(m, gState.selIdx);
                }
                break;

            case KEY_DOWN:
                if (gState.selIdx < tab->itemCount - 1) {
                    int oldIdx = gState.selIdx;
                    gState.selIdx++;
                    drawItemAt(m, oldIdx);
                    drawItemAt(m, gState.selIdx);
                } else {
                    int oldIdx = gState.selIdx;
                    gState.selIdx = 0;
                    drawItemAt(m, oldIdx);
                    drawItemAt(m, gState.selIdx);
                }
                break;

            case KEY_LEFT:
                if (m->tabCount > 1) {
                    gState.activeTab--;
                    if (gState.activeTab < 0) gState.activeTab = m->tabCount - 1;
                    gState.selIdx = m->tabs[gState.activeTab].initialSel;
                    if (gState.selIdx >= m->tabs[gState.activeTab].itemCount)
                        gState.selIdx = 0;
                    drawMenuContent(m);
                }
                break;

            case KEY_RIGHT:
                if (m->tabCount > 1) {
                    gState.activeTab++;
                    if (gState.activeTab >= m->tabCount) gState.activeTab = 0;
                    gState.selIdx = m->tabs[gState.activeTab].initialSel;
                    if (gState.selIdx >= m->tabs[gState.activeTab].itemCount)
                        gState.selIdx = 0;
                    drawMenuContent(m);
                }
                break;

            case KEY_ENTER:
                gState.confirmed = 1;
                gState.running = 0;
                break;

            case KEY_ESC:
                gState.running = 0;
                break;

            case KEY_MOUSE:
                if (ev.mouseBtn == 0 || ev.mouseBtn == 2) {
                    int t = hitTestTab(ev.mouseX, ev.mouseY, m);
                    if (t >= 0 && t != gState.activeTab) {
                        gState.activeTab = t;
                        gState.selIdx = m->tabs[t].initialSel;
                        if (gState.selIdx >= m->tabs[t].itemCount)
                            gState.selIdx = 0;
                        drawMenuContent(m);
                        break;
                    }
                    int idx = hitTestItem(ev.mouseX, ev.mouseY, m);
                    if (idx >= 0) {
                        gState.selIdx = idx;
                        gState.confirmed = 1;
                        gState.running = 0;
                    }
                }
                break;

            case KEY_NONE:
            default:
                break;
        }
    }

    if (m->useMouse) printf(MOUSE_OFF);
    if (m->useAltBuffer) printf(ALT_BUF_OFF);
    termRestore();
    sigaction(SIGWINCH, &oldSa, NULL);

    if (gState.confirmed) {
        res.selectedTab = gState.activeTab;
        res.selectedItem = gState.selIdx;
    }
    return res;
}
