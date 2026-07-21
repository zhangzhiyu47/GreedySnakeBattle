/*
 * declarative_button.c
 * Production-grade, dependency-free C TUI button button.
 * Background: RGB(255, 250, 240) warm ivory.
 *
 * Features:
 *   - Title (colored text-only), Hint, vertical button list
 *   - Bottom dual buttons (left/right, individually optional)
 *   - When bottom enabled: top = select, bottom = confirm/cancel
 *   - When bottom disabled: top = direct confirm
 *   - Tab switches bottom button focus
 *   - Scrollable content, full border, wide char support
 *   - Zero dependencies, pure ANSI
 *
 * Compile: gcc -O2 -o button_test declarative_button.c button_test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <locale.h>

#include "include/button.h"

#define ESC          "\033["
#define ALT_BUF_ON   ESC "?1049h"
#define ALT_BUF_OFF  ESC "?1049l"
#define CLEAR_SCREEN ESC "H" ESC "2J" ESC "3J"
#define MOUSE_ON     ESC "?1000h" ESC "?1002h" ESC "?1015h" ESC "?1006h"
#define MOUSE_OFF    ESC "?1006l" ESC "?1015l" ESC "?1002l" ESC "?1000l"
#define RESET_COLOR  ESC "0m"

/* ANSI helpers */
void fgRgb(int r, int g, int b) {
    printf(ESC "38;2;%d;%d;%dm", r, g, b);
}
void bgRgb(int r, int g, int b) {
    printf(ESC "48;2;%d;%d;%dm", r, g, b);
}
void moveCursor(int row, int col) {
    printf(ESC "%d;%dH", row, col);
}

/* Default palette */
static const Color cBg            = {255, 250, 240}; /* warm ivory */
static const Color cBorder        = {160, 174, 192}; /* soft steel */
static const Color cShadow        = {200, 190, 180}; /* warm grey */
static const Color cTitle         = {59, 130, 246};  /* bright blue */
static const Color cHint          = {113, 128, 150}; /* muted blue-grey */
static const Color cBtnFg         = {45, 55, 72};    /* dark slate */
static const Color cBtnBg         = {255, 250, 240}; /* warm ivory */
static const Color cBtnSelFg      = {255, 255, 255}; /* white */
static const Color cBtnSelBg      = {59, 130, 246};  /* bright blue */
static const Color cBottomFg      = {45, 55, 72};    /* dark slate */
static const Color cBottomBg      = {240, 245, 250}; /* light blue-grey */
static const Color cBottomSelFg   = {255, 255, 255}; /* white */
static const Color cBottomSelBg   = {37, 99, 235};   /* darker blue */

/* Single top button */
typedef struct {
    char *label;
} TopButton;

/* Button configuration and state */
struct Button {
    char *titleStr;
    char *hintStr;
    TopButton *buttons;
    int btnCount;
    int btnCap;
    char *bottomLeft;
    char *bottomRight;
    int hasBottomLeft;
    int hasBottomRight;

    Color bg, fg, border, shadow, titleCol, hintCol;
    Color btnFg, btnBg, btnSelFg, btnSelBg;
    Color bottomFg, bottomBg, bottomSelFg, bottomSelBg;
    int hasBg, hasFg, hasBorder, hasShadow, hasTitleCol, hasHintCol;
    int hasBtnFg, hasBtnBg, hasBtnSelFg, hasBtnSelBg;
    int hasBottomFg, hasBottomBg, hasBottomSelFg, hasBottomSelBg;

    int shadowEnabled;
    int borderDouble;
    int roundedCorners;
    int useAltBuffer;
    int useMouse;
    int initialSel;
    int x, y, w, h;
    ButtonBgDrawFn bgDrawFn;
    void *bgDrawData;
};

/* Internal runtime state */
typedef struct {
    int termW, termH;
    int btnX, btnY;
    int btnW, btnH;
    int selIdx;          /* selected top button */
    int bottomFocus;     /* 0 = left, 1 = right, -1 = none */
    int scrollOffset;    /* scroll offset for top buttons */
    int running;
    int confirmed;
    int cancelled;
    struct termios origTerm;
} ButtonState;

static ButtonState gState;
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

void putUtf8(int codepoint) {
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

/*
 * Display width of a UTF-8 string.
 * ASCII = 1, CJK/wide = 2, combining = 0.
 */
static int displayWidth(const char *s) {
    int w = 0;
    const unsigned char *p = (const unsigned char *)s;
    while (*p) {
        if (*p < 0x80) {
            w++;
            p++;
        } else if ((*p & 0xE0) == 0xC0) {
            w += 1;
            p += 2;
        } else if ((*p & 0xF0) == 0xE0) {
            unsigned int cp = ((*p & 0x0F) << 12) |
                              ((p[1] & 0x3F) << 6) |
                              (p[2] & 0x3F);
            if ((cp >= 0x2E80 && cp <= 0x9FFF) ||
                (cp >= 0xAC00 && cp <= 0xD7FF) ||
                (cp >= 0xF900 && cp <= 0xFAFF) ||
                (cp >= 0xFE30 && cp <= 0xFE4F) ||
                (cp >= 0xFF00 && cp <= 0xFFEF) ||
                (cp >= 0x20000 && cp <= 0x2FA1F)) {
                w += 2;
            } else {
                w += 1;
            }
            p += 3;
        } else if ((*p & 0xF8) == 0xF0) {
            unsigned int cp = ((*p & 0x07) << 18) |
                              ((p[1] & 0x3F) << 12) |
                              ((p[2] & 0x3F) << 6) |
                              (p[3] & 0x3F);
            if (cp >= 0x1F000 && cp <= 0x1FFFF) {
                w += 2;
            } else {
                w += 1;
            }
            p += 4;
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

static void fillRect(int r, int c, int h, int w, const Color *bg) {
    int i, j;
    for (i = 0; i < h; i++) {
        moveCursor(r + i, c);
        if (bg) bgRgb(bg->r, bg->g, bg->b);
        for (j = 0; j < w; j++) putchar(' ');
    }
}

static void drawShadow(int r, int c, int h, int w, const Color *shadow) {
    int i;
    const Color *s = shadow ? shadow : &cShadow;
    Color dim;
    dim.r = (s->r + cBg.r) / 2;
    dim.g = (s->g + cBg.g) / 2;
    dim.b = (s->b + cBg.b) / 2;
    for (i = 1; i < h && (r + i) <= gState.termH; i++) {
        if (c + w <= gState.termW)
            drawCharAt(r + i, c + w, 0x2591, &dim, NULL);
    }
    for (i = 0; i < w && (r + h) <= gState.termH; i++) {
        if (c + i + 1 <= gState.termW)
            drawCharAt(r + h, c + i + 1, 0x2591, &dim, NULL);
    }
    if ((r + h) <= gState.termH && (c + w) <= gState.termW)
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

/*
 * Draw title: centered, colored text only (no background color on text),
 * but the area behind title is filled with button background.
 */
static void drawTitle(int r, int c, int w, const Button *d) {
    const Color *bg = d->hasBg ? &d->bg : &cBg;
    const Color *fg = d->hasTitleCol ? &d->titleCol : &cTitle;
    int titleW = displayWidth(d->titleStr);
    int startX = c + (w - titleW) / 2;
    if (startX < c + 1) startX = c + 1;

    /* Fill title row background */
    moveCursor(r, c + 1);
    bgRgb(bg->r, bg->g, bg->b);
    int i;
    for (i = 0; i < w - 2; i++) putchar(' ');

    /* Draw title text with colored foreground, no background override */
    moveCursor(r, startX);
    fgRgb(fg->r, fg->g, fg->b);
    printf("%s", d->titleStr);
    printf(RESET_COLOR);
}

/*
 * Draw hint: centered, default foreground, below title.
 */
static void drawHint(int r, int c, int w, const Button *d) {
    const Color *bg = d->hasBg ? &d->bg : &cBg;
    const Color *fg = d->hasHintCol ? &d->hintCol : &cHint;
    int hintW = displayWidth(d->hintStr);
    int startX = c + (w - hintW) / 2;
    if (startX < c + 1) startX = c + 1;

    /* Fill hint row background */
    moveCursor(r, c + 1);
    bgRgb(bg->r, bg->g, bg->b);
    int i;
    for (i = 0; i < w - 2; i++) putchar(' ');

    /* Draw hint text */
    moveCursor(r, startX);
    fgRgb(fg->r, fg->g, fg->b);
    printf("%s", d->hintStr);
    printf(RESET_COLOR);
}

/*
 * Draw a single top button at given row.
 * isSelected: whether this button is currently highlighted.
 */
static void drawTopButton(int row, int c, int w,
                           const Button *d, int idx, int isSelected) {
    const Color *btnBg   = d->hasBtnBg   ? &d->btnBg   : &cBtnBg;
    const Color *btnFg   = d->hasBtnFg   ? &d->btnFg   : &cBtnFg;
    const Color *selBg   = d->hasBtnSelBg ? &d->btnSelBg : &cBtnSelBg;
    const Color *selFg   = d->hasBtnSelFg ? &d->btnSelFg : &cBtnSelFg;

    const Color *itemBg = isSelected ? selBg : btnBg;
    const Color *itemFg = isSelected ? selFg : btnFg;
    const char *label = d->buttons[idx].label;
    int labelW = displayWidth(label);
    int availW = w - 4; /* 2 padding each side */
    int padLeft = 2;
    int padRight = availW - labelW;
    if (padRight < 0) padRight = 0;

    moveCursor(row, c + 1);
    bgRgb(itemBg->r, itemBg->g, itemBg->b);
    fgRgb(itemFg->r, itemFg->g, itemFg->b);

    int p;
    for (p = 0; p < padLeft; p++) putchar(' ');
    printf("%s", label);
    for (p = 0; p < padRight; p++) putchar(' ');

    /* Fill remaining width if label was shorter than availW */
    int totalContent = padLeft + labelW + padRight;
    int remaining = (w - 2) - totalContent;
    for (p = 0; p < remaining; p++) putchar(' ');

    printf(RESET_COLOR);
}

/*
 * Draw separator line between content and bottom buttons.
 */
static void drawSeparator(int r, int c, int w, const Button *d) {
    const Color *bg = d->hasBg ? &d->bg : &cBg;
    const Color *bdr = d->hasBorder ? &d->border : &cBorder;
    moveCursor(r, c + 1);
    bgRgb(bg->r, bg->g, bg->b);
    fgRgb(bdr->r, bdr->g, bdr->b);
    int i;
    for (i = 0; i < w - 2; i++) putUtf8(0x2500);
    printf(RESET_COLOR);
}

/*
 * Draw bottom button bar.
 * leftLabel/rightLabel may be NULL.
 * bottomFocus: 0=left, 1=right, -1=none.
 */
static void drawBottomBar(int r, int c, int w,
                           const Button *d) {
    const Color *bg        = d->hasBg ? &d->bg : &cBg;
    const Color *btmBg     = d->hasBottomBg ? &d->bottomBg : &cBottomBg;
    const Color *btmFg     = d->hasBottomFg ? &d->bottomFg : &cBottomFg;
    const Color *selBg     = d->hasBottomSelBg ? &d->bottomSelBg : &cBottomSelBg;
    const Color *selFg     = d->hasBottomSelFg ? &d->bottomSelFg : &cBottomSelFg;

    int hasLeft = d->hasBottomLeft && d->bottomLeft;
    int hasRight = d->hasBottomRight && d->bottomRight;
    int leftW = hasLeft ? displayWidth(d->bottomLeft) + 4 : 0;  /* [ text ] */
    int rightW = hasRight ? displayWidth(d->bottomRight) + 4 : 0;
    int totalBtnW = leftW + rightW;
    if (hasLeft && hasRight) totalBtnW += 2; /* gap between buttons */

    int startX = c + (w - totalBtnW) / 2;
    if (startX < c + 1) startX = c + 1;

    /* Fill entire bottom row background */
    moveCursor(r, c + 1);
    bgRgb(bg->r, bg->g, bg->b);
    int i;
    for (i = 0; i < w - 2; i++) putchar(' ');

    int currentX = startX;

    /* Left button */
    if (hasLeft) {
        int isSel = (gState.bottomFocus == 0);
        const Color *itemBg = isSel ? selBg : btmBg;
        const Color *itemFg = isSel ? selFg : btmFg;

        moveCursor(r, currentX);
        bgRgb(itemBg->r, itemBg->g, itemBg->b);
        fgRgb(itemFg->r, itemFg->g, itemFg->b);
        putchar(' ');
        putchar('[');
        printf("%s", d->bottomLeft);
        putchar(']');
        putchar(' ');
        printf(RESET_COLOR);
        currentX += leftW;
    }

    /* Gap */
    if (hasLeft && hasRight) {
        moveCursor(r, currentX);
        bgRgb(bg->r, bg->g, bg->b);
        putchar(' ');
        putchar(' ');
        printf(RESET_COLOR);
        currentX += 2;
    }

    /* Right button */
    if (hasRight) {
        int isSel = (gState.bottomFocus == 1);
        const Color *itemBg = isSel ? selBg : btmBg;
        const Color *itemFg = isSel ? selFg : btmFg;

        moveCursor(r, currentX);
        bgRgb(itemBg->r, itemBg->g, itemBg->b);
        fgRgb(itemFg->r, itemFg->g, itemFg->b);
        putchar(' ');
        putchar('[');
        printf("%s", d->bottomRight);
        putchar(']');
        putchar(' ');
        printf(RESET_COLOR);
    }
}

/*
 * Calculate button geometry.
 */
static void buttonInitGeometry(const Button *d) {
    int maxLabelW = 0;
    int i;
    int titleW = d->titleStr ? displayWidth(d->titleStr) : 0;
    int hintW = d->hintStr ? displayWidth(d->hintStr) : 0;

    for (i = 0; i < d->btnCount; i++) {
        int lw = displayWidth(d->buttons[i].label);
        if (lw > maxLabelW) maxLabelW = lw;
    }

    int bottomW = 0;
    if (d->hasBottomLeft && d->bottomLeft)
        bottomW += displayWidth(d->bottomLeft) + 4;
    if (d->hasBottomRight && d->bottomRight)
        bottomW += displayWidth(d->bottomRight) + 4;
    if (d->hasBottomLeft && d->hasBottomRight) bottomW += 2;

    int contentW = maxLabelW;
    if (titleW > contentW) contentW = titleW;
    if (hintW > contentW) contentW = hintW;
    if (bottomW > contentW) contentW = bottomW;

    gState.btnW = d->w;
    if (gState.btnW == 0) {
        gState.btnW = contentW + 8; /* padding + borders */
        if (gState.btnW < 24) gState.btnW = 24;
        if (gState.btnW > gState.termW - 4)
            gState.btnW = gState.termW - 4;
    }

    int hasBottom = (d->hasBottomLeft || d->hasBottomRight);
    int minContentH = 3;
    int maxContentH = gState.termH - 8;
    if (hasBottom) maxContentH -= 2;
    int contentH = d->btnCount;
    if (contentH < minContentH) contentH = minContentH;
    if (contentH > maxContentH) contentH = maxContentH;

    gState.btnH = 1 + 1 + 1 + 1 + contentH + 1 + (hasBottom ? 2 : 0) + 1;
    if (gState.btnH > gState.termH - 2)
        gState.btnH = gState.termH - 2;

    gState.btnX = d->x;
    gState.btnY = d->y;
    if (gState.btnX == 0) gState.btnX = (gState.termW - gState.btnW) / 2;
    if (gState.btnY == 0) gState.btnY = (gState.termH - gState.btnH) / 2;
    if (gState.btnX < 1) gState.btnX = 1;
    if (gState.btnY < 1) gState.btnY = 1;

    gState.selIdx = d->initialSel;
    if (gState.selIdx < 0) gState.selIdx = 0;
    if (gState.selIdx >= d->btnCount) gState.selIdx = 0;

    if (d->hasBottomLeft) {
        gState.bottomFocus = 0;
    } else if (d->hasBottomRight) {
        gState.bottomFocus = 1;
    } else {
        gState.bottomFocus = -1;
    }

    gState.scrollOffset = 0;
}

/*
 * Full redraw.
 */
static void drawButtonFull(const Button *d) {
    int r = gState.btnY;
    int c = gState.btnX;
    int w = gState.btnW;
    int h = gState.btnH;
    const Color *bg = d->hasBg ? &d->bg : &cBg;
    int hasBottom = (d->hasBottomLeft || d->hasBottomRight);
    int contentH = h - (hasBottom ? 7 : 5);
    int i;

    /* User background callback: paint the whole screen first */
    if (d->bgDrawFn) {
        d->bgDrawFn(gState.termW, gState.termH, d->bgDrawData);
    } else {
        fillRect(r, c, h, w, bg);
    }

    if (d->shadowEnabled) {
        const Color *sh = d->hasShadow ? &d->shadow : &cShadow;
        drawShadow(r, c, h, w, sh);
    }

    drawBorder(r, c, h, w, d->hasBorder ? &d->border : NULL,
                d->borderDouble, d->roundedCorners);

    if (d->titleStr && d->titleStr[0]) {
        drawTitle(r + 1, c, w, d);
    }

    if (d->hintStr && d->hintStr[0]) {
        drawHint(r + 2, c, w, d);
    }

    int btnStartRow = r + 4;
    for (i = 0; i < contentH; i++) {
        int btnIdx = gState.scrollOffset + i;
        if (btnIdx < d->btnCount) {
            drawTopButton(btnStartRow + i, c, w, d, btnIdx,
                           btnIdx == gState.selIdx);
        } else {
            moveCursor(btnStartRow + i, c + 1);
            bgRgb(bg->r, bg->g, bg->b);
            int p;
            for (p = 0; p < w - 2; p++) putchar(' ');
            printf(RESET_COLOR);
        }
    }

    if (gState.scrollOffset > 0) {
        moveCursor(btnStartRow, c + w - 2);
        bgRgb(bg->r, bg->g, bg->b);
        fgRgb(cBorder.r, cBorder.g, cBorder.b);
        putUtf8(0x25B2);
        printf(RESET_COLOR);
    } else {
        moveCursor(btnStartRow, c + w - 2);
        bgRgb(bg->r, bg->g, bg->b);
        putchar(' ');
        printf(RESET_COLOR);
    }
    if (gState.scrollOffset + contentH < d->btnCount) {
        moveCursor(btnStartRow + contentH - 1, c + w - 2);
        bgRgb(bg->r, bg->g, bg->b);
        fgRgb(cBorder.r, cBorder.g, cBorder.b);
        putUtf8(0x25BC);
        printf(RESET_COLOR);
    } else {
        moveCursor(btnStartRow + contentH - 1, c + w - 2);
        bgRgb(bg->r, bg->g, bg->b);
        putchar(' ');
        printf(RESET_COLOR);
    }

    if (hasBottom) {
        int sepRow = r + h - 3;
        int btmRow = r + h - 2;
        drawSeparator(sepRow, c, w, d);
        drawBottomBar(btmRow, c, w, d);
    }

    fflush(stdout);
}

/*
 * Redraw only the content area (buttons).
 */
static void drawButtonContent(const Button *d) {
    int r = gState.btnY;
    int c = gState.btnX;
    int w = gState.btnW;
    int h = gState.btnH;
    const Color *bg = d->hasBg ? &d->bg : &cBg;
    int hasBottom = (d->hasBottomLeft || d->hasBottomRight);
    int contentH = h - (hasBottom ? 7 : 5);
    int btnStartRow = r + 4;
    int i;

    for (i = 0; i < contentH; i++) {
        int btnIdx = gState.scrollOffset + i;
        if (btnIdx < d->btnCount) {
            drawTopButton(btnStartRow + i, c, w, d, btnIdx,
                           btnIdx == gState.selIdx);
        } else {
            moveCursor(btnStartRow + i, c + 1);
            bgRgb(bg->r, bg->g, bg->b);
            int p;
            for (p = 0; p < w - 2; p++) putchar(' ');
            printf(RESET_COLOR);
        }
    }

    if (gState.scrollOffset > 0) {
        moveCursor(btnStartRow, c + w - 2);
        bgRgb(bg->r, bg->g, bg->b);
        fgRgb(cBorder.r, cBorder.g, cBorder.b);
        putUtf8(0x25B2);
        printf(RESET_COLOR);
    } else {
        moveCursor(btnStartRow, c + w - 2);
        bgRgb(bg->r, bg->g, bg->b);
        putchar(' ');
        printf(RESET_COLOR);
    }
    if (gState.scrollOffset + contentH < d->btnCount) {
        moveCursor(btnStartRow + contentH - 1, c + w - 2);
        bgRgb(bg->r, bg->g, bg->b);
        fgRgb(cBorder.r, cBorder.g, cBorder.b);
        putUtf8(0x25BC);
        printf(RESET_COLOR);
    } else {
        moveCursor(btnStartRow + contentH - 1, c + w - 2);
        bgRgb(bg->r, bg->g, bg->b);
        putchar(' ');
        printf(RESET_COLOR);
    }

    fflush(stdout);
}

/*
 * Redraw only a single button row.
 */
static void drawButtonAt(const Button *d, int idx) {
    int r = gState.btnY;
    int c = gState.btnX;
    int w = gState.btnW;
    int h = gState.btnH;
    int hasBottom = (d->hasBottomLeft || d->hasBottomRight);
    int contentH = h - (hasBottom ? 7 : 5);
    int btnStartRow = r + 4;

    int rowInView = idx - gState.scrollOffset;
    if (rowInView < 0 || rowInView >= contentH) return;

    drawTopButton(btnStartRow + rowInView, c, w, d, idx,
                   idx == gState.selIdx);
    fflush(stdout);
}

/*
 * Redraw bottom bar only.
 */
static void drawBottomBarOnly(const Button *d) {
    int r = gState.btnY;
    int c = gState.btnX;
    int w = gState.btnW;
    int h = gState.btnH;
    int btmRow = r + h - 2;
    drawBottomBar(btmRow, c, w, d);
    fflush(stdout);
}

/*
 * Ensure selected item is in view.
 */
static void buttonEnsureVisible(const Button *d) {
    int hasBottom = (d->hasBottomLeft || d->hasBottomRight);
    int contentH = gState.btnH - (hasBottom ? 7 : 5);

    if (gState.selIdx < gState.scrollOffset) {
        gState.scrollOffset = gState.selIdx;
    } else if (gState.selIdx >= gState.scrollOffset + contentH) {
        gState.scrollOffset = gState.selIdx - contentH + 1;
    }
}

/* Input event types */
#define KEY_UP       1
#define KEY_DOWN     2
#define KEY_LEFT     3
#define KEY_RIGHT    4
#define KEY_ENTER    5
#define KEY_ESC      6
#define KEY_TAB      7
#define KEY_MOUSE    8
#define KEY_NONE     0

typedef struct {
    int type;
    int mouseX;
    int mouseY;
    int mouseBtn;
    int mousePress;
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
            if (c3 == 'Z') { ev.type = KEY_TAB; return ev; }

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

        if (c2 == 'O') {
            int c3 = readByte();
            if (c3 == 'Z') { ev.type = KEY_TAB; return ev; }
        }
    }

    if (c == 9) { ev.type = KEY_TAB; return ev; }

    if (c == 'w' || c == 'W' || c == 'k' || c == 'K') { ev.type = KEY_UP; return ev; }
    if (c == 's' || c == 'S' || c == 'j' || c == 'J') { ev.type = KEY_DOWN; return ev; }
    if (c == 'a' || c == 'A' || c == 'h' || c == 'H') { ev.type = KEY_LEFT; return ev; }
    if (c == 'd' || c == 'D' || c == 'l' || c == 'L') { ev.type = KEY_RIGHT; return ev; }

    if (c == 10 || c == 13) { ev.type = KEY_ENTER; return ev; }

    if (c == 3 || c == 4 || c == 'q' || c == 'Q') { ev.type = KEY_ESC; return ev; }

    return ev;
}

/*
 * Hit test for top buttons.
 */
static int hitTestTopItem(int mx, int my, const Button *d) {
    int r = gState.btnY;
    int c = gState.btnX;
    int h = gState.btnH;
    int hasBottom = (d->hasBottomLeft || d->hasBottomRight);
    int contentH = h - (hasBottom ? 7 : 5);
    int btnStartRow = r + 4;

    if (mx < c + 1 || mx >= c + gState.btnW - 1) return -1;
    if (my < btnStartRow || my >= btnStartRow + contentH) return -1;

    int idx = gState.scrollOffset + (my - btnStartRow);
    if (idx < 0 || idx >= d->btnCount) return -1;
    return idx;
}

/*
 * Hit test for bottom buttons.
 * Returns: 0 = left, 1 = right, -1 = none.
 */
static int hitTestBottomItem(int mx, int my, const Button *d) {
    int r = gState.btnY;
    int c = gState.btnX;
    int w = gState.btnW;
    int h = gState.btnH;
    int btmRow = r + h - 2;

    if (my != btmRow) return -1;
    if (mx < c + 1 || mx >= c + w - 1) return -1;

    int hasLeft = d->hasBottomLeft && d->bottomLeft;
    int hasRight = d->hasBottomRight && d->bottomRight;
    int leftW = hasLeft ? displayWidth(d->bottomLeft) + 4 : 0;
    int rightW = hasRight ? displayWidth(d->bottomRight) + 4 : 0;
    int totalBtnW = leftW + rightW;
    if (hasLeft && hasRight) totalBtnW += 2;

    int startX = c + (w - totalBtnW) / 2;
    if (startX < c + 1) startX = c + 1;

    if (hasLeft) {
        if (mx >= startX && mx < startX + leftW) return 0;
    }
    if (hasRight) {
        int rightStart = startX + leftW + (hasLeft ? 2 : 0);
        if (mx >= rightStart && mx < rightStart + rightW) return 1;
    }
    return -1;
}

/* ================================================================== */
/* Public API                                                         */
/* ================================================================== */

Button* buttonCreate(void) {
    Button *d = calloc(1, sizeof(Button));
    if (!d) return NULL;
    d->btnCap = 8;
    d->buttons = calloc(d->btnCap, sizeof(TopButton));
    if (!d->buttons) { free(d); return NULL; }
    d->shadowEnabled = 1;
    d->borderDouble = 0;
    d->roundedCorners = 1;
    d->useAltBuffer = 1;
    d->useMouse = 1;
    d->initialSel = 0;
    return d;
}

void buttonFree(Button *d) {
    int i;
    if (!d) return;
    free(d->titleStr);
    free(d->hintStr);
    for (i = 0; i < d->btnCount; i++) {
        free(d->buttons[i].label);
    }
    free(d->buttons);
    free(d->bottomLeft);
    free(d->bottomRight);
    free(d);
}

void buttonTitle(Button *d, const char *title) {
    if (!d) return;
    free(d->titleStr);
    d->titleStr = title ? strdup(title) : NULL;
}

void buttonHint(Button *d, const char *hint) {
    if (!d) return;
    free(d->hintStr);
    d->hintStr = hint ? strdup(hint) : NULL;
}

int buttonAdd(Button *b, const char *label) {
    if (!b || !label) return -1;
    if (b->btnCount >= b->btnCap) {
        int newCap = b->btnCap * 2;
        TopButton *newBtns = realloc(b->buttons, newCap * sizeof(TopButton));
        if (!newBtns) return -1;
        memset(newBtns + b->btnCap, 0, (newCap - b->btnCap) * sizeof(TopButton));
        b->buttons = newBtns;
        b->btnCap = newCap;
    }
    b->buttons[b->btnCount].label = strdup(label);
    return b->btnCount++;
}

void buttonBottomLeft(Button *d, const char *label) {
    if (!d) return;
    free(d->bottomLeft);
    d->bottomLeft = label ? strdup(label) : NULL;
    d->hasBottomLeft = (label != NULL);
}

void buttonBottomRight(Button *d, const char *label) {
    if (!d) return;
    free(d->bottomRight);
    d->bottomRight = label ? strdup(label) : NULL;
    d->hasBottomRight = (label != NULL);
}

void buttonColor(Button *d, int which, Color c) {
    if (!d) return;
    switch (which) {
        case BUTTON_COLOR_BG:            d->bg = c; d->hasBg = 1; break;
        case BUTTON_COLOR_FG:            d->fg = c; d->hasFg = 1; break;
        case BUTTON_COLOR_BORDER:        d->border = c; d->hasBorder = 1; break;
        case BUTTON_COLOR_SHADOW:        d->shadow = c; d->hasShadow = 1; break;
        case BUTTON_COLOR_TITLE:         d->titleCol = c; d->hasTitleCol = 1; break;
        case BUTTON_COLOR_HINT:          d->hintCol = c; d->hasHintCol = 1; break;
        case BUTTON_COLOR_BTN_FG:        d->btnFg = c; d->hasBtnFg = 1; break;
        case BUTTON_COLOR_BTN_BG:        d->btnBg = c; d->hasBtnBg = 1; break;
        case BUTTON_COLOR_BTN_SEL_FG:    d->btnSelFg = c; d->hasBtnSelFg = 1; break;
        case BUTTON_COLOR_BTN_SEL_BG:    d->btnSelBg = c; d->hasBtnSelBg = 1; break;
        case BUTTON_COLOR_BOTTOM_FG:     d->bottomFg = c; d->hasBottomFg = 1; break;
        case BUTTON_COLOR_BOTTOM_BG:     d->bottomBg = c; d->hasBottomBg = 1; break;
        case BUTTON_COLOR_BOTTOM_SEL_FG: d->bottomSelFg = c; d->hasBottomSelFg = 1; break;
        case BUTTON_COLOR_BOTTOM_SEL_BG: d->bottomSelBg = c; d->hasBottomSelBg = 1; break;
    }
}

void buttonStyle(Button *d, int shadow, int doubleBorder, int rounded) {
    if (!d) return;
    d->shadowEnabled = shadow;
    d->borderDouble = doubleBorder;
    d->roundedCorners = rounded;
}

void buttonPos(Button *d, int x, int y, int w, int h) {
    if (!d) return;
    d->x = x; d->y = y; d->w = w; d->h = h;
}

void buttonInitial(Button *d, int idx) {
    if (!d) return;
    d->initialSel = idx;
}

void buttonUseAltBuffer(Button *d, int use) {
    if (!d) return;
    d->useAltBuffer = use;
}

void buttonUseMouse(Button *d, int use) {
    if (!d) return;
    d->useMouse = use;
}

/* Set user background draw callback. Pass NULL to disable. */
void buttonBgDraw(Button *d, ButtonBgDrawFn fn, void *userData) {
    if (!d) return;
    d->bgDrawFn = fn;
    d->bgDrawData = userData;
}

/* ================================================================== */
/* buttonRun                                                          */
/* ================================================================== */

ButtonResult buttonRun(Button *d) {
    ButtonResult res = {-1, 0, -1};
    InputEvent ev;
    struct sigaction sa, oldSa;
    int hasBottom;

    if (!d) return res;

    memset(&gState, 0, sizeof(gState));
    getTermSize();
    buttonInitGeometry(d);

    hasBottom = (d->hasBottomLeft || d->hasBottomRight);

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = onResize;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGWINCH, &sa, &oldSa);

    if (d->useAltBuffer) printf(ALT_BUF_ON);
    printf(CLEAR_SCREEN);
    if (d->useMouse) printf(MOUSE_ON);
    termRaw();

    gState.running = 1;
    gState.confirmed = 0;
    gState.cancelled = 0;

    printf(CLEAR_SCREEN);
    drawButtonFull(d);

    while (gState.running) {
        if (gResized) {
            gResized = 0;
            getTermSize();
            buttonInitGeometry(d);
            printf(CLEAR_SCREEN);
            drawButtonFull(d);
        }

        ev = parseInput();

        switch (ev.type) {
            case KEY_UP:
                if (d->btnCount > 0) {
                    int oldIdx = gState.selIdx;
                    if (gState.selIdx > 0) {
                        gState.selIdx--;
                    } else {
                        gState.selIdx = d->btnCount - 1;
                    }
                    buttonEnsureVisible(d);
                    drawButtonAt(d, oldIdx);
                    drawButtonAt(d, gState.selIdx);
                }
                break;

            case KEY_DOWN:
                if (d->btnCount > 0) {
                    int oldIdx = gState.selIdx;
                    if (gState.selIdx < d->btnCount - 1) {
                        gState.selIdx++;
                    } else {
                        gState.selIdx = 0;
                    }
                    buttonEnsureVisible(d);
                    drawButtonAt(d, oldIdx);
                    drawButtonAt(d, gState.selIdx);
                }
                break;

            case KEY_LEFT:
                if (hasBottom && gState.bottomFocus >= 0) {
                    if (d->hasBottomLeft) {
                        gState.bottomFocus = 0;
                        drawBottomBarOnly(d);
                    }
                }
                break;

            case KEY_RIGHT:
                if (hasBottom && gState.bottomFocus >= 0) {
                    if (d->hasBottomRight) {
                        gState.bottomFocus = 1;
                        drawBottomBarOnly(d);
                    }
                }
                break;

            case KEY_TAB:
                if (hasBottom) {
                    if (d->hasBottomLeft && d->hasBottomRight) {
                        gState.bottomFocus = (gState.bottomFocus == 0) ? 1 : 0;
                    } else if (d->hasBottomLeft) {
                        gState.bottomFocus = 0;
                    } else if (d->hasBottomRight) {
                        gState.bottomFocus = 1;
                    }
                    drawBottomBarOnly(d);
                }
                break;

            case KEY_ENTER:
                if (hasBottom) {
                    if (gState.bottomFocus == 0 && d->hasBottomLeft) {
                        gState.confirmed = 1;
                        gState.running = 0;
                        res.bottomButton = 0;
                    } else if (gState.bottomFocus == 1 && d->hasBottomRight) {
                        gState.confirmed = 1;
                        gState.running = 0;
                        res.bottomButton = 1;
                    }
                } else {
                    if (d->btnCount > 0) {
                        gState.confirmed = 1;
                        gState.running = 0;
                    }
                }
                break;

            case KEY_ESC:
                gState.cancelled = 1;
                gState.running = 0;
                break;

            case KEY_MOUSE:
                if (ev.mouseBtn == 0 || ev.mouseBtn == 2) {
                    int topIdx = hitTestTopItem(ev.mouseX, ev.mouseY, d);
                    if (topIdx >= 0) {
                        gState.selIdx = topIdx;
                        buttonEnsureVisible(d);
                        if (hasBottom) {
                            drawButtonContent(d);
                        } else {
                            gState.confirmed = 1;
                            gState.running = 0;
                        }
                        break;
                    }

                    if (hasBottom) {
                        int btm = hitTestBottomItem(ev.mouseX, ev.mouseY, d);
                        if (btm == 0) {
                            gState.confirmed = 1;
                            gState.running = 0;
                            res.bottomButton = 0;
                        } else if (btm == 1) {
                            gState.confirmed = 1;
                            gState.running = 0;
                            res.bottomButton = 1;
                        }
                    }
                }
                break;

            case KEY_NONE:
            default:
                break;
        }
    }

    if (d->useMouse) printf(MOUSE_OFF);
    if (d->useAltBuffer) printf(ALT_BUF_OFF);
    termRestore();
    sigaction(SIGWINCH, &oldSa, NULL);

    res.selectedTop = gState.selIdx;
    res.confirmed = gState.confirmed;
    if (!gState.confirmed && !gState.cancelled) {
        res.confirmed = 0;
    }
    return res;
}
