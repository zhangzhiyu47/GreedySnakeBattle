/*
 * declarative_browser.c
 * Production-grade, dependency-free C TUI text browser.
 * Background: RGB(255, 250, 240) warm ivory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <locale.h>

#include "include/menu.h"
#include "include/browser.h"

#define ESC          "\033["
#define ALT_BUF_ON   ESC "?1049h"
#define ALT_BUF_OFF  ESC "?1049l"
#define CLEAR_SCREEN ESC "H" ESC "2J" ESC "3J"

static void fgRgb(int r, int g, int b) {
    printf(ESC "38;2;%d;%d;%dm", r, g, b);
}
static void bgRgb(int r, int g, int b) {
    printf(ESC "48;2;%d;%d;%dm", r, g, b);
}
static void moveCursor(int row, int col) {
    printf(ESC "%d;%dH", row, col);
}

static const Color cBg        = {255, 250, 240};
static const Color cFg        = {45, 55, 72};
static const Color cHintBg    = {59, 130, 246};
static const Color cHintFg    = {255, 255, 255};
static const Color cPromptFg  = {113, 128, 150};
static const Color cBorder    = {160, 174, 192};
static const Color cBtnBg     = {226, 232, 240};
static const Color cBtnFg     = {45, 55, 72};
static const Color cBtnHlBg   = {59, 130, 246};
static const Color cBtnHlFg   = {255, 255, 255};

/* Text line */
typedef struct {
    char *text;
    int width;
    int len;
} TextLine;

struct TextBrowser {
    char *hintText;
    char *promptText;
    TextLine *lines;
    int lineCount;
    int lineCap;
    char *btnLeft;
    char *btnRight;
    int x, y, w, h;
    Color bg, fg, hintBg, hintFg, promptFg, border, btnBg, btnFg, btnHlBg, btnHlFg;
    int hasBg, hasFg, hasHintBg, hasHintFg, hasPromptFg, hasBorder;
    int hasBtnBg, hasBtnFg, hasBtnHlBg, hasBtnHlFg;
    int borderEnabled;
    int roundedCorners;
    int useAltBuffer;
    int useMouse;
};

/* Layout */
typedef struct {
    int termW, termH;
    int bx, by, bw, bh;
    int hintR, hintR2;       /* hint row 1 and optional row 2, -1 if none */
    int promptR;
    int boxTopR, boxInnerR, boxInnerH, boxInnerW, boxBottomR;
    int btnR;
    int scrollX, scrollY;
    int selBtn;
    int running, confirmed;
    struct termios origTerm;
} BrowserState;

static BrowserState gState;
static volatile sig_atomic_t gResized = 0;

static void onResize(int sig) { (void)sig; gResized = 1; }

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

/* Display width: ASCII=1, CJK=2 */
static int displayWidth(const char *s) {
    int w = 0;
    const unsigned char *p = (const unsigned char *)s;
    while (*p) {
        if (*p < 0x80) { w++; p++; }
        else if ((*p & 0xE0) == 0xC0) { w += 2; p += 2; }
        else if ((*p & 0xF0) == 0xE0) { w += 2; p += 3; }
        else if ((*p & 0xF8) == 0xF0) { w += 2; p += 4; }
        else { p++; }
    }
    return w;
}

static int utf8CharLen(const unsigned char *p) {
    if (*p < 0x80) return 1;
    if ((*p & 0xE0) == 0xC0) return 2;
    if ((*p & 0xF0) == 0xE0) return 3;
    if ((*p & 0xF8) == 0xF0) return 4;
    return 1;
}

static void putUtf8(int cp) {
    if (cp <= 0x7F) putchar(cp);
    else if (cp <= 0x7FF) { putchar(0xC0 | (cp >> 6)); putchar(0x80 | (cp & 0x3F)); }
    else if (cp <= 0xFFFF) { putchar(0xE0 | (cp >> 12)); putchar(0x80 | ((cp >> 6) & 0x3F)); putchar(0x80 | (cp & 0x3F)); }
    else { putchar(0xF0 | (cp >> 18)); putchar(0x80 | ((cp >> 12) & 0x3F)); putchar(0x80 | ((cp >> 6) & 0x3F)); putchar(0x80 | (cp & 0x3F)); }
}

/* Split text by \n */
static void parseText(TextBrowser *b, const char *text) {
    const char *p, *start;
    int count = 0, i;
    for (i = 0; i < b->lineCount; i++) free(b->lines[i].text);
    b->lineCount = 0;
    if (!text || !*text) return;
    p = text; while (*p) { if (*p == '\n') count++; p++; } count++;
    if (count > b->lineCap) {
        int nc = count * 2;
        TextLine *nl = realloc(b->lines, nc * sizeof(TextLine));
        if (!nl) return;
        memset(nl + b->lineCap, 0, (nc - b->lineCap) * sizeof(TextLine));
        b->lines = nl; b->lineCap = nc;
    }
    p = text; start = text;
    while (1) {
        if (*p == '\n' || *p == '\0') {
            int len = p - start;
            char *line = malloc(len + 1);
            if (line) {
                memcpy(line, start, len); line[len] = '\0';
                b->lines[b->lineCount].text = line;
                b->lines[b->lineCount].len = len;
                b->lines[b->lineCount].width = displayWidth(line);
                b->lineCount++;
            }
            if (*p == '\0') break;
            start = p + 1;
        }
        p++;
    }
}

/* ================================================================== */
/* Drawing primitives                                                   */
/* ================================================================== */

static void fillSpaces(int n) { while (n-- > 0) putchar(' '); }

static void drawSpaces(int row, int col, int n, const Color *bg) {
    moveCursor(row, col);
    if (bg) bgRgb(bg->r, bg->g, bg->b);
    fillSpaces(n);
}

/* Draw text line with scroll offset. Wide char boundary: skip if doesn't fit. */
static void drawTextLine(int row, int col, int maxW, const char *text,
                         int scrollX, const Color *bg, const Color *fg) {
    const unsigned char *p = (const unsigned char *)text;
    int x = 0, printed = 0;
    moveCursor(row, col);
    if (bg) bgRgb(bg->r, bg->g, bg->b);
    if (fg) fgRgb(fg->r, fg->g, fg->b);
    while (*p && printed < maxW) {
        int cl = utf8CharLen(p);
        int cw = (cl == 1 && *p < 0x80) ? 1 : 2;
        if (x < scrollX) { x += cw; p += cl; continue; }
        if (printed + cw > maxW) { fillSpaces(maxW - printed); break; }
        int j; for (j = 0; j < cl; j++) putchar(p[j]);
        printed += cw; p += cl;
    }
    fillSpaces(maxW - printed);
}

/* ================================================================== */
/* Hint wrapping: returns number of lines (1 or 2)                    */
/* ================================================================== */

static int wrapHint(const char *text, int maxW, char **line1, char **line2) {
    int w = displayWidth(text);
    *line1 = NULL; *line2 = NULL;
    if (w <= maxW) {
        *line1 = strdup(text);
        return 1;
    }
    /* Try to wrap at space */
    const char *p = text;
    int cw = 0;
    const char *lastSpace = NULL;
    int lastSpaceByte = 0;
    while (*p) {
        int cl = utf8CharLen((const unsigned char *)p);
        int ccw = (cl == 1 && *p < 0x80) ? 1 : 2;
        if (*p == ' ') { lastSpace = p; lastSpaceByte = cw; }
        if (cw + ccw > maxW) break;
        cw += ccw; p += cl;
    }
    if (lastSpace && lastSpaceByte > maxW / 3) {
        int l1len = lastSpace - text;
        *line1 = malloc(l1len + 1);
        memcpy(*line1, text, l1len); (*line1)[l1len] = '\0';
        *line2 = strdup(lastSpace + 1);
    } else {
        /* Hard wrap: skip leading spaces on line 2 */
        int l1len = p - text;
        *line1 = malloc(l1len + 1);
        memcpy(*line1, text, l1len); (*line1)[l1len] = '\0';
        const char *lp = p;
        while (*lp == ' ') lp++;
        *line2 = strdup(lp);
    }
    /* Truncate line2 if too long */
    if (*line2 && displayWidth(*line2) > maxW) {
        int need = maxW;
        char *lp = *line2;
        char *trunc = malloc(strlen(*line2) + 4);
        int tw = 0, tl = 0;
        while (*lp && tw < need - 1) {
            int cl = utf8CharLen((const unsigned char *)lp);
            int ccw = (cl == 1 && *lp < 0x80) ? 1 : 2;
            if (tw + ccw > need - 1) break;
            int j; for (j = 0; j < cl; j++) trunc[tl++] = *lp++;
            tw += ccw;
        }
        trunc[tl++] = '.'; trunc[tl++] = '.'; trunc[tl++] = '.';
        trunc[tl] = '\0';
        free(*line2);
        *line2 = trunc;
    }
    return 2;
}

/* ================================================================== */
/* Component drawing                                                    */
/* ================================================================== */

/* Draw hint line at row, with blue bg on text only */
static void drawHintLine(const TextBrowser *b, int row, const char *text, int maxW) {
    const Color *bg = b->hasBg ? &b->bg : &cBg;
    const Color *hbg = b->hasHintBg ? &b->hintBg : &cHintBg;
    const Color *hfg = b->hasHintFg ? &b->hintFg : &cHintFg;
    int hw = displayWidth(text);
    int pad = (maxW - hw) / 2;
    if (pad < 0) pad = 0;
    int after = maxW - pad - hw;
    if (after < 0) after = 0;

    moveCursor(row, gState.bx + 1);
    bgRgb(bg->r, bg->g, bg->b);
    fillSpaces(pad);
    bgRgb(hbg->r, hbg->g, hbg->b);
    fgRgb(hfg->r, hfg->g, hfg->b);
    printf("%s", text);
    bgRgb(bg->r, bg->g, bg->b);
    fgRgb(hfg->r, hfg->g, hfg->b);
    fillSpaces(after);
}

/* Draw hint (supports 1 or 2 lines) */
static void drawHint(const TextBrowser *b) {
    if (!b->hintText || !*b->hintText) return;
    int maxW = gState.bw - 2;
    char *l1 = NULL, *l2 = NULL;
    int lines = wrapHint(b->hintText, maxW, &l1, &l2);
    if (l1) {
        drawHintLine(b, gState.hintR, l1, maxW);
        free(l1);
    }
    if (l2 && lines == 2 && gState.hintR2 >= 0) {
        drawHintLine(b, gState.hintR2, l2, maxW);
        free(l2);
    } else if (l2) {
        free(l2);
    }
}

/* Prompt: muted color text, left-aligned inside outer border */
static void drawPrompt(const TextBrowser *b) {
    if (!b->promptText || !*b->promptText) return;
    const Color *bg = b->hasBg ? &b->bg : &cBg;
    const Color *fg = b->hasPromptFg ? &b->promptFg : &cPromptFg;
    int pw = displayWidth(b->promptText);
    int maxW = gState.bw - 2;
    moveCursor(gState.promptR, gState.bx + 1);
    bgRgb(bg->r, bg->g, bg->b);
    fgRgb(fg->r, fg->g, fg->b);
    printf("%s", b->promptText);
    fillSpaces(maxW - pw);
}

/* Draw outer border segment */
static void drawOuterBorder(const TextBrowser *b) {
    const Color *br = b->hasBorder ? &b->border : &cBorder;
    const Color *bg = b->hasBg ? &b->bg : &cBg;
    int w = gState.bw;
    int left = gState.bx;
    int right = gState.bx + w - 1;
    int top = gState.by;
    int bot = gState.by + gState.bh - 1;
    int rounded = b->roundedCorners;
    int i;

    /* Top */
    moveCursor(top, left);
    bgRgb(bg->r, bg->g, bg->b);
    fgRgb(br->r, br->g, br->b);
    putUtf8(rounded ? 0x256D : 0x250C);
    for (i = 1; i < w - 1; i++) putUtf8(0x2500);
    putUtf8(rounded ? 0x256E : 0x2510);

    /* Sides */
    for (i = top + 1; i < bot; i++) {
        moveCursor(i, left);
        bgRgb(bg->r, bg->g, bg->b);
        fgRgb(br->r, br->g, br->b);
        putUtf8(0x2502);
        moveCursor(i, right);
        putUtf8(0x2502);
    }

    /* Bottom */
    moveCursor(bot, left);
    bgRgb(bg->r, bg->g, bg->b);
    fgRgb(br->r, br->g, br->b);
    putUtf8(rounded ? 0x2570 : 0x2514);
    for (i = 1; i < w - 1; i++) putUtf8(0x2500);
    putUtf8(rounded ? 0x256F : 0x2518);
}

/* Content box inner borders (inside outer border) */
static void drawInnerBoxBorders(const TextBrowser *b) {
    const Color *br = b->hasBorder ? &b->border : &cBorder;
    const Color *bg = b->hasBg ? &b->bg : &cBg;
    int w = gState.bw - 4;
    int left = gState.bx + 2;
    int right = gState.bx + gState.bw - 3;
    int top = gState.boxTopR;
    int bot = gState.boxBottomR;
    int rounded = b->roundedCorners;
    int i;

    /* Top inner border */
    moveCursor(top, left);
    bgRgb(bg->r, bg->g, bg->b);
    fgRgb(br->r, br->g, br->b);
    putUtf8(rounded ? 0x256D : 0x250C);
    for (i = 1; i < w - 1; i++) putUtf8(0x2500);
    putUtf8(rounded ? 0x256E : 0x2510);

    /* Side inner borders */
    for (i = gState.boxInnerR; i < bot; i++) {
        moveCursor(i, left);
        bgRgb(bg->r, bg->g, bg->b);
        fgRgb(br->r, br->g, br->b);
        putUtf8(0x2502);
        moveCursor(i, right);
        putUtf8(0x2502);
    }

    /* Bottom inner border */
    moveCursor(bot, left);
    bgRgb(bg->r, bg->g, bg->b);
    fgRgb(br->r, br->g, br->b);
    putUtf8(rounded ? 0x2570 : 0x2514);
    for (i = 1; i < w - 1; i++) putUtf8(0x2500);
    putUtf8(rounded ? 0x256F : 0x2518);
}

/* Draw one content row */
static void drawContentRow(const TextBrowser *b, int offset) {
    const Color *bg = b->hasBg ? &b->bg : &cBg;
    const Color *fg = b->hasFg ? &b->fg : &cFg;
    int idx = gState.scrollY + offset;
    int row = gState.boxInnerR + offset;
    int col = gState.bx + 3;
    int w = gState.boxInnerW;
    if (idx < b->lineCount) {
        drawTextLine(row, col, w, b->lines[idx].text, gState.scrollX, bg, fg);
    } else {
        drawSpaces(row, col, w, bg);
    }
}

/* Draw all content rows */
static void drawContentAll(const TextBrowser *b) {
    int i;
    for (i = 0; i < gState.boxInnerH; i++) drawContentRow(b, i);
}

/* Vertical scrollbar inside inner box */
static void drawVScrollBar(const TextBrowser *b) {
    int total = b->lineCount;
    int vis = gState.boxInnerH;
    if (total <= vis || vis <= 0) return;
    const Color *br = b->hasBorder ? &b->border : &cBorder;
    const Color *bg = b->hasBg ? &b->bg : &cBg;
    int trackH = vis;
    int thumbH = vis * vis / total;
    if (thumbH < 1) thumbH = 1;
    int thumbPos = gState.scrollY * (trackH - thumbH) / (total - vis);
    if (thumbPos < 0) thumbPos = 0;
    if (thumbPos > trackH - thumbH) thumbPos = trackH - thumbH;

    int col = gState.bx + gState.bw - 4;
    int i;
    for (i = 0; i < trackH; i++) {
        moveCursor(gState.boxInnerR + i, col);
        bgRgb(bg->r, bg->g, bg->b);
        if (i >= thumbPos && i < thumbPos + thumbH) {
            fgRgb(br->r, br->g, br->b);
            putUtf8(0x2588);
        } else {
            fgRgb(br->r, br->g, br->b);
            putUtf8(0x2502);
        }
    }
}

/* Horizontal scrollbar on bottom inner border row */
static void drawHScrollBar(const TextBrowser *b) {
    int i, maxW = 0;
    for (i = 0; i < b->lineCount; i++)
        if (b->lines[i].width > maxW) maxW = b->lines[i].width;
    int maxScroll = maxW - gState.boxInnerW;
    if (maxScroll <= 0) return;

    const Color *br = b->hasBorder ? &b->border : &cBorder;
    const Color *bg = b->hasBg ? &b->bg : &cBg;
    int trackW = gState.boxInnerW;
    int totalW = gState.boxInnerW + maxScroll;
    int thumbW = gState.boxInnerW * trackW / totalW;
    if (thumbW < 2) thumbW = 2;
    int thumbPos = gState.scrollX * (trackW - thumbW) / maxScroll;
    if (thumbPos < 0) thumbPos = 0;
    if (thumbPos > trackW - thumbW) thumbPos = trackW - thumbW;

    int row = gState.boxBottomR;
    int col = gState.bx + 3;
    moveCursor(row, col);
    bgRgb(bg->r, bg->g, bg->b);
    for (i = 0; i < trackW; i++) {
        if (i >= thumbPos && i < thumbPos + thumbW) {
            fgRgb(br->r, br->g, br->b);
            putUtf8(0x2588);
        } else {
            fgRgb(br->r, br->g, br->b);
            putUtf8(0x2500);
        }
    }
}

/* Draw button */
static void drawButton(int r, int c, int w, const char *label, int sel, const TextBrowser *b) {
    const Color *bg = b->hasBg ? &b->bg : &cBg;
    const Color *bbg = sel ? (b->hasBtnHlBg ? &b->btnHlBg : &cBtnHlBg) : (b->hasBtnBg ? &b->btnBg : &cBtnBg);
    const Color *bfg = sel ? (b->hasBtnHlFg ? &b->btnHlFg : &cBtnHlFg) : (b->hasBtnFg ? &b->btnFg : &cBtnFg);
    int lw = label ? displayWidth(label) : 0;
    int pl = (w - lw) / 2;
    int pr = w - lw - pl;
    moveCursor(r, c);
    bgRgb(bg->r, bg->g, bg->b);
    fillSpaces(pl);
    bgRgb(bbg->r, bbg->g, bbg->b);
    fgRgb(bfg->r, bfg->g, bfg->b);
    if (label) printf("%s", label);
    bgRgb(bg->r, bg->g, bg->b);
    fgRgb(bfg->r, bfg->g, bfg->b);
    fillSpaces(pr);
}

/* Draw buttons */
static void drawButtons(const TextBrowser *b) {
    int hasL = (b->btnLeft && *b->btnLeft);
    int hasR = (b->btnRight && *b->btnRight);
    if (!hasL && !hasR) return;

    int bw = 16;
    if (bw > gState.bw / 2 - 6) bw = gState.bw / 2 - 6;
    if (bw < 8) bw = 8;
    int cnt = (hasL ? 1 : 0) + (hasR ? 1 : 0);
    int gap, lc, rc;

    if (cnt == 2) {
        gap = (gState.bw - bw * 2) / 3;
        lc = gState.bx + gap;
        rc = lc + bw + gap;
    } else {
        gap = (gState.bw - bw) / 2;
        lc = gState.bx + gap;
        rc = lc;
    }

    /* Always draw both to clear previous highlight */
    if (hasL) drawButton(gState.btnR, lc, bw, b->btnLeft, gState.selBtn == 0, b);
    if (hasR && cnt == 2) drawButton(gState.btnR, rc, bw, b->btnRight, gState.selBtn == 1, b);
    else if (hasR && cnt == 1) drawButton(gState.btnR, lc, bw, b->btnRight, gState.selBtn == 1, b);
}

/* ================================================================== */
/* Full redraw and smooth scroll                                        */
/* ================================================================== */

static void drawBrowserFull(const TextBrowser *b) {
    const Color *bg = b->hasBg ? &b->bg : &cBg;
    int i;

    /* Clear outer area with background color */
    for (i = 0; i < gState.bh; i++) {
        moveCursor(gState.by + i, gState.bx);
        bgRgb(bg->r, bg->g, bg->b);
        fillSpaces(gState.bw);
    }

    /* Draw outer border */
    if (b->borderEnabled) drawOuterBorder(b);

    /* Hint */
    if (gState.hintR >= 0) drawHint(b);

    /* Prompt */
    if (gState.promptR >= 0) drawPrompt(b);

    /* Inner box borders */
    if (b->borderEnabled) drawInnerBoxBorders(b);

    /* Content text */
    drawContentAll(b);

    /* Scrollbars */
    drawVScrollBar(b);
    drawHScrollBar(b);

    /* Buttons */
    if (gState.btnR >= 0) drawButtons(b);

    fflush(stdout);
}

/* Smooth scroll: only redraw content + scrollbars */
static void drawContentSmooth(const TextBrowser *b) {
    drawContentAll(b);
    drawVScrollBar(b);
    drawHScrollBar(b);
    fflush(stdout);
}

/* ================================================================== */
/* Input                                                                */
/* ================================================================== */

#define KEY_UP 1
#define KEY_DOWN 2
#define KEY_LEFT 3
#define KEY_RIGHT 4
#define KEY_ENTER 5
#define KEY_ESC 6
#define KEY_MOUSE 7
#define KEY_PGUP 8
#define KEY_PGDN 9
#define KEY_HOME 10
#define KEY_END 11
#define KEY_TAB 12
#define KEY_NONE 0

typedef struct { int type; int mouseX, mouseY, mouseBtn, mousePress; } InputEvent;

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
            if (c3 == '5') { if (readByte() == '~') { ev.type = KEY_PGUP; return ev; } }
            if (c3 == '6') { if (readByte() == '~') { ev.type = KEY_PGDN; return ev; } }
            if (c3 == '1') { if (readByte() == '~') { ev.type = KEY_HOME; return ev; } }
            if (c3 == '4') { if (readByte() == '~') { ev.type = KEY_END; return ev; } }
            if (c3 == 'H') { ev.type = KEY_HOME; return ev; }
            if (c3 == 'F') { ev.type = KEY_END; return ev; }
            if (c3 == 'Z') { ev.type = KEY_TAB; return ev; }
            if (c3 == '<') {
                int btn = 0, mx = 0, my = 0, n;
                char buf[32];
                int i = 0, lastCh = 0;
                while (i < 31) {
                    int ch = readByte();
                    if (ch < 0) break;
                    buf[i++] = (char)ch;
                    if (ch == 'M' || ch == 'm') { lastCh = ch; break; }
                }
                buf[i] = '\0';
                n = sscanf(buf, "%d;%d;%d", &btn, &mx, &my);
                if (n >= 3) {
                    ev.type = KEY_MOUSE;
                    ev.mouseX = mx; ev.mouseY = my;
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
    if (c == 'g') { ev.type = KEY_HOME; return ev; }
    if (c == 'G') { ev.type = KEY_END; return ev; }
    if (c == '\t') { ev.type = KEY_TAB; return ev; }
    if (c == 10 || c == 13) { ev.type = KEY_ENTER; return ev; }
    if (c == 3 || c == 4 || c == 'q' || c == 'Q') { ev.type = KEY_ESC; return ev; }
    return ev;
}

static int hitTestButton(int mx, int my, const TextBrowser *b) {
    int hasL = (b->btnLeft && *b->btnLeft);
    int hasR = (b->btnRight && *b->btnRight);
    if (!hasL && !hasR) return -1;
    if (my != gState.btnR) return -1;

    int bw = 16;
    if (bw > gState.bw / 2 - 6) bw = gState.bw / 2 - 6;
    if (bw < 8) bw = 8;
    int cnt = (hasL ? 1 : 0) + (hasR ? 1 : 0);
    int gap, lc, rc;
    if (cnt == 2) {
        gap = (gState.bw - bw * 2) / 3;
        lc = gState.bx + gap;
        rc = lc + bw + gap;
    } else {
        gap = (gState.bw - bw) / 2;
        lc = gState.bx + gap;
        rc = lc;
    }
    if (hasL && mx >= lc && mx < lc + bw) return 0;
    if (hasR && cnt == 2 && mx >= rc && mx < rc + bw) return 1;
    if (hasR && cnt == 1 && mx >= lc && mx < lc + bw) return 1;
    return -1;
}

/* ================================================================== */
/* Geometry                                                             */
/* ================================================================== */

static int getMaxScrollX(const TextBrowser *b) {
    int i, maxW = 0;
    for (i = 0; i < b->lineCount; i++)
        if (b->lines[i].width > maxW) maxW = b->lines[i].width;
    int ms = maxW - gState.boxInnerW;
    return (ms > 0) ? ms : 0;
}

static void browserInitGeometry(TextBrowser *b) {
    int maxW = 0, i;
    int hasHint = (b->hintText && *b->hintText);
    int hintLines = 1;
    int hasPrompt = (b->promptText && *b->promptText);
    int hasL = (b->btnLeft && *b->btnLeft);
    int hasR = (b->btnRight && *b->btnRight);
    int hasButtons = (hasL || hasR);

    getTermSize();

    for (i = 0; i < b->lineCount; i++)
        if (b->lines[i].width > maxW) maxW = b->lines[i].width;

    /* Outer width */
    gState.bw = b->w;
    if (gState.bw == 0) {
        gState.bw = maxW + 10;
        if (hasHint && displayWidth(b->hintText) + 6 > gState.bw)
            gState.bw = displayWidth(b->hintText) + 6;
        if (hasPrompt && displayWidth(b->promptText) + 6 > gState.bw)
            gState.bw = displayWidth(b->promptText) + 6;
        if (gState.bw < 28) gState.bw = 28;
        if (gState.bw > gState.termW - 2) gState.bw = gState.termW - 2;
    }

    /* Calculate hint lines */
    if (hasHint) {
        int hw = displayWidth(b->hintText);
        int maxHintW = gState.bw - 2;
        if (hw > maxHintW) {
            hintLines = 2;
            /* If even 2 lines not enough, we will truncate in wrapHint */
        }
    }

    /* Outer height */
    gState.bh = b->h;
    if (gState.bh == 0) {
        gState.bh = 7; /* outer borders(2) + inner borders(2) + padding(2) + 1 content */
        if (hasHint) gState.bh += hintLines;
        if (hasPrompt) gState.bh += 1;
        gState.bh += b->lineCount;
        if (hasButtons) gState.bh += 1;
        if (gState.bh > gState.termH - 1) gState.bh = gState.termH - 1;
        if (gState.bh < 8) gState.bh = 8;
    }

    gState.bx = b->x ? b->x : (gState.termW - gState.bw) / 2;
    gState.by = b->y ? b->y : (gState.termH - gState.bh) / 2;
    if (gState.bx < 1) gState.bx = 1;
    if (gState.by < 1) gState.by = 1;

    /* Assign rows */
    int row = gState.by + 1; /* +1 for top outer border */
    gState.hintR = hasHint ? row++ : -1;
    gState.hintR2 = (hasHint && hintLines == 2) ? row++ : -1;
    gState.promptR = hasPrompt ? row++ : -1;
    gState.boxTopR = row++; /* inner box top border */
    gState.boxInnerR = row;
    /* boxInnerH = remaining - inner bottom border - outer bottom border - optional button */
    int used = row - gState.by;
    gState.boxInnerH = gState.bh - used - 2; /* -2 for inner bottom + outer bottom */
    if (hasButtons) gState.boxInnerH -= 1;
    if (gState.boxInnerH < 1) gState.boxInnerH = 1;
    row += gState.boxInnerH;
    gState.boxBottomR = row++; /* inner box bottom border */
    if (hasButtons) gState.btnR = row++;
    else gState.btnR = -1;

    /* Inner width: total - 2 outer borders - 2 inner borders - 2 padding - 1 scrollbar */
    gState.boxInnerW = gState.bw - 7;
    if (gState.boxInnerW < 6) gState.boxInnerW = 6;

    /* Clamp scroll */
    int maxScrollY = b->lineCount - gState.boxInnerH;
    if (maxScrollY < 0) maxScrollY = 0;
    if (gState.scrollY > maxScrollY) gState.scrollY = maxScrollY;
    int maxScrollX = getMaxScrollX(b);
    if (gState.scrollX > maxScrollX) gState.scrollX = maxScrollX;
}

/* ================================================================== */
/* Public API                                                           */
/* ================================================================== */

TextBrowser* browserCreate(void) {
    TextBrowser *b = calloc(1, sizeof(TextBrowser));
    if (!b) return NULL;
    b->lineCap = 16;
    b->lines = calloc(b->lineCap, sizeof(TextLine));
    if (!b->lines) { free(b); return NULL; }
    b->borderEnabled = 1;
    b->roundedCorners = 1;
    b->useAltBuffer = 1;
    b->useMouse = 1;
    return b;
}

void browserFree(TextBrowser *b) {
    int i;
    if (!b) return;
    free(b->hintText);
    free(b->promptText);
    for (i = 0; i < b->lineCount; i++) free(b->lines[i].text);
    free(b->lines);
    free(b->btnLeft);
    free(b->btnRight);
    free(b);
}

void browserHint(TextBrowser *b, const char *hint) {
    if (!b) return;
    free(b->hintText);
    b->hintText = hint ? strdup(hint) : NULL;
}

void browserPrompt(TextBrowser *b, const char *prompt) {
    if (!b) return;
    free(b->promptText);
    b->promptText = prompt ? strdup(prompt) : NULL;
}

void browserText(TextBrowser *b, const char *text) {
    if (!b) return;
    parseText(b, text);
}

void browserButtonLeft(TextBrowser *b, const char *label) {
    if (!b) return;
    free(b->btnLeft);
    b->btnLeft = label ? strdup(label) : NULL;
}

void browserButtonRight(TextBrowser *b, const char *label) {
    if (!b) return;
    free(b->btnRight);
    b->btnRight = label ? strdup(label) : NULL;
}

void browserColor(TextBrowser *b, int which, Color c) {
    if (!b) return;
    switch (which) {
        case BROWSER_COLOR_BG:        b->bg = c; b->hasBg = 1; break;
        case BROWSER_COLOR_FG:        b->fg = c; b->hasFg = 1; break;
        case BROWSER_COLOR_HINT_BG:   b->hintBg = c; b->hasHintBg = 1; break;
        case BROWSER_COLOR_HINT_FG:   b->hintFg = c; b->hasHintFg = 1; break;
        case BROWSER_COLOR_PROMPT_FG: b->promptFg = c; b->hasPromptFg = 1; break;
        case BROWSER_COLOR_BORDER:    b->border = c; b->hasBorder = 1; break;
        case BROWSER_COLOR_BUTTON_BG: b->btnBg = c; b->hasBtnBg = 1; break;
        case BROWSER_COLOR_BUTTON_FG: b->btnFg = c; b->hasBtnFg = 1; break;
        case BROWSER_COLOR_BUTTON_HL_BG: b->btnHlBg = c; b->hasBtnHlBg = 1; break;
        case BROWSER_COLOR_BUTTON_HL_FG: b->btnHlFg = c; b->hasBtnHlFg = 1; break;
    }
}

void browserStyle(TextBrowser *b, int border, int rounded) {
    if (!b) return;
    b->borderEnabled = border;
    b->roundedCorners = rounded;
}

void browserPos(TextBrowser *b, int x, int y, int w, int h) {
    if (!b) return;
    b->x = x; b->y = y; b->w = w; b->h = h;
}

void browserUseAltBuffer(TextBrowser *b, int use) {
    if (!b) return;
    b->useAltBuffer = use;
}

void browserUseMouse(TextBrowser *b, int use) {
    if (!b) return;
    b->useMouse = use;
}

BrowserResult browserRun(TextBrowser *b) {
    BrowserResult res = {-1};
    InputEvent ev;
    struct sigaction sa, oldSa;
    int hasL, hasR, hasButtons;
    int maxScrollY, maxScrollX;

    if (!b) return res;

    memset(&gState, 0, sizeof(gState));
    gState.selBtn = 0;
    browserInitGeometry(b);

    hasL = (b->btnLeft && *b->btnLeft);
    hasR = (b->btnRight && *b->btnRight);
    hasButtons = (hasL || hasR);
    if (!hasL && hasR) gState.selBtn = 1;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = onResize;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGWINCH, &sa, &oldSa);

    if (b->useAltBuffer) printf(ALT_BUF_ON);
    printf(CLEAR_SCREEN);
    termRaw();

    gState.running = 1;
    gState.confirmed = 0;

    printf(CLEAR_SCREEN);
    drawBrowserFull(b);

    while (gState.running) {
        if (gResized) {
            gResized = 0;
            getTermSize();
            browserInitGeometry(b);
            printf(CLEAR_SCREEN);
            drawBrowserFull(b);
        }

        ev = parseInput();

        maxScrollY = b->lineCount - gState.boxInnerH;
        if (maxScrollY < 0) maxScrollY = 0;
        maxScrollX = getMaxScrollX(b);

        switch (ev.type) {
            case KEY_UP:
                if (gState.scrollY > 0) { gState.scrollY--; drawContentSmooth(b); }
                break;
            case KEY_DOWN:
                if (gState.scrollY < maxScrollY) { gState.scrollY++; drawContentSmooth(b); }
                break;
            case KEY_LEFT:
                if (gState.scrollX > 0) {
                    gState.scrollX -= 4;
                    if (gState.scrollX < 0) gState.scrollX = 0;
                    drawContentSmooth(b);
                }
                break;
            case KEY_RIGHT:
                if (gState.scrollX < maxScrollX) {
                    gState.scrollX += 4;
                    if (gState.scrollX > maxScrollX) gState.scrollX = maxScrollX;
                    drawContentSmooth(b);
                }
                break;
            case KEY_PGUP:
                gState.scrollY -= gState.boxInnerH;
                if (gState.scrollY < 0) gState.scrollY = 0;
                drawContentSmooth(b);
                break;
            case KEY_PGDN:
                gState.scrollY += gState.boxInnerH;
                if (gState.scrollY > maxScrollY) gState.scrollY = maxScrollY;
                drawContentSmooth(b);
                break;
            case KEY_HOME:
                gState.scrollY = 0; gState.scrollX = 0;
                drawContentSmooth(b);
                break;
            case KEY_END:
                gState.scrollY = maxScrollY;
                drawContentSmooth(b);
                break;
            case KEY_TAB:
                if (hasButtons) {
                    int old = gState.selBtn;
                    if (hasL && hasR) {
                        gState.selBtn = !gState.selBtn;
                    }
                    if (old != gState.selBtn) drawButtons(b);
                }
                break;
            case KEY_ENTER:
                if (hasButtons) { gState.confirmed = 1; gState.running = 0; }
                break;
            case KEY_ESC:
                gState.running = 0;
                break;
            case KEY_MOUSE:
                if (ev.mousePress) {
                    if (ev.mouseBtn == 64) {
                        if (gState.scrollY > 0) { gState.scrollY--; drawContentSmooth(b); }
                        break;
                    }
                    if (ev.mouseBtn == 65) {
                        if (gState.scrollY < maxScrollY) { gState.scrollY++; drawContentSmooth(b); }
                        break;
                    }
                    if (ev.mouseBtn == 0 || ev.mouseBtn == 2) {
                        int btn = hitTestButton(ev.mouseX, ev.mouseY, b);
                        if (btn >= 0) {
                            gState.selBtn = btn;
                            gState.confirmed = 1;
                            gState.running = 0;
                        }
                    }
                }
                break;
            default: break;
        }
    }

    if (b->useAltBuffer) printf(ALT_BUF_OFF);
    termRestore();
    sigaction(SIGWINCH, &oldSa, NULL);

    if (gState.confirmed) res.selectedButton = gState.selBtn;
    return res;
}
