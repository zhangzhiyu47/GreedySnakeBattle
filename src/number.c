/*
 * declarative_number.c
 * Production-grade, dependency-free C TUI number dialog.
 * Background: RGB(255, 250, 240) warm ivory.
 *
 * Features:
 *   - Title (blue text-only), Hint, vertical number field list
 *   - Each field: description + [-] value [+], with min/max/decimals/step
 *   - Enter to toggle input mode (type number directly)
 *   - Bottom dual buttons (left/right, individually optional, at least one)
 *   - When bottom enabled: top fields = navigate/select, bottom = confirm/cancel
 *   - When bottom disabled: top fields = direct confirm on Enter
 *   - Tab switches bottom button focus
 *   - Scrollable content, full border, wide char support
 *   - Zero dependencies, pure ANSI
 *
 * Compile: gcc -O2 -o number_test declarative_number.c number_test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <locale.h>
#include <math.h>

#include "include/number.h"

#define ESC          "\033["
#define ALT_BUF_ON   ESC "?1049h"
#define ALT_BUF_OFF  ESC "?1049l"
#define CLEAR_SCREEN ESC "H" ESC "2J" ESC "3J"
#define MOUSE_ON     ESC "?1000h" ESC "?1002h" ESC "?1015h" ESC "?1006h"
#define MOUSE_OFF    ESC "?1006l" ESC "?1015l" ESC "?1002l" ESC "?1000l"
#define RESET_COLOR  ESC "0m"

/* ANSI helpers - public */
void numMoveCursor(int row, int col) {
    printf(ESC "%d;%dH", row, col);
}
void numBgRgb(int r, int g, int b) {
    printf(ESC "48;2;%d;%d;%dm", r, g, b);
}
void numFgRgb(int r, int g, int b) {
    printf(ESC "38;2;%d;%d;%dm", r, g, b);
}
void numPutUtf8(int codepoint) {
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

/* Default palette */
static const NumColor cBg            = {255, 250, 240}; /* warm ivory */
static const NumColor cBorder        = {160, 174, 192}; /* soft steel */
static const NumColor cShadow        = {200, 190, 180}; /* warm grey */
static const NumColor cTitle         = {59, 130, 246};  /* bright blue */
static const NumColor cHint          = {113, 128, 150}; /* muted blue-grey */
static const NumColor cDescFg        = {45, 55, 72};    /* dark slate */
static const NumColor cValueFg       = {37, 99, 235};   /* darker blue */
static const NumColor cAdjustFg      = {113, 128, 150}; /* muted blue-grey */
static const NumColor cAdjustBg      = {255, 250, 240}; /* warm ivory */
static const NumColor cAdjustSelFg   = {255, 255, 255}; /* white */
static const NumColor cAdjustSelBg   = {59, 130, 246};  /* bright blue */
static const NumColor cBottomFg      = {45, 55, 72};    /* dark slate */
static const NumColor cBottomBg      = {240, 245, 250}; /* light blue-grey */
static const NumColor cBottomSelFg   = {255, 255, 255}; /* white */
static const NumColor cBottomSelBg   = {37, 99, 235};   /* darker blue */
static const NumColor cInputBg       = {254, 252, 232}; /* light yellow */
static const NumColor cInputFg       = {220, 38, 38};   /* red */

/* Single number field */
struct NumberField {
    char *desc;
    double minVal;
    double maxVal;
    double value;
    int decimals;
    double step;
};

/* Number dialog configuration and state */
struct NumberDialog {
    char *titleStr;
    char *hintStr;
    NumberField *fields;
    int fieldCount;
    int fieldCap;
    char *bottomLeft;
    char *bottomRight;
    int hasBottomLeft;
    int hasBottomRight;

    NumColor bg, fg, border, shadow, titleCol, hintCol;
    NumColor descFg, valueFg;
    NumColor adjustFg, adjustBg, adjustSelFg, adjustSelBg;
    NumColor bottomFg, bottomBg, bottomSelFg, bottomSelBg;
    NumColor inputBg, inputFg;
    int hasBg, hasFg, hasBorder, hasShadow, hasTitleCol, hasHintCol;
    int hasDescFg, hasValueFg;
    int hasAdjustFg, hasAdjustBg, hasAdjustSelFg, hasAdjustSelBg;
    int hasBottomFg, hasBottomBg, hasBottomSelFg, hasBottomSelBg;
    int hasInputBg, hasInputFg;

    int shadowEnabled;
    int borderDouble;
    int roundedCorners;
    int useAltBuffer;
    int useMouse;
    int initialSel;
    int x, y, w, h;
    NumberBgDrawFn bgDrawFn;
    void *bgDrawData;
};

/* Internal runtime state */
typedef struct {
    int termW, termH;
    int dlgX, dlgY;
    int dlgW, dlgH;
    int selIdx;          /* selected field */
    int bottomFocus;     /* 0 = left, 1 = right, -1 = none */
    int scrollOffset;    /* scroll offset for fields */
    int inInputMode;     /* 1 = typing input, 0 = normal */
    char inputBuf[64];
    int inputLen;
    int running;
    int confirmed;
    int cancelled;
    struct termios origTerm;
} NumberState;

static NumberState gState;
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

static void drawCharAt(int row, int col, int codepoint, const NumColor *bg, const NumColor *fg) {
    numMoveCursor(row, col);
    if (bg) numBgRgb(bg->r, bg->g, bg->b);
    if (fg) numFgRgb(fg->r, fg->g, fg->b);
    numPutUtf8(codepoint);
}

static void fillRect(int r, int c, int h, int w, const NumColor *bg) {
    int i, j;
    for (i = 0; i < h; i++) {
        numMoveCursor(r + i, c);
        if (bg) numBgRgb(bg->r, bg->g, bg->b);
        for (j = 0; j < w; j++) putchar(' ');
    }
}

static void drawShadow(int r, int c, int h, int w, const NumColor *shadow) {
    int i;
    const NumColor *s = shadow ? shadow : &cShadow;
    NumColor dim;
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
                        const NumColor *border, int doubleLine, int rounded) {
    int i;
    const NumColor *b = border ? border : &cBorder;
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

    numFgRgb(b->r, b->g, b->b);

    numMoveCursor(r, c);
    numPutUtf8(tl);
    for (i = 0; i < w - 2; i++) numPutUtf8(hz);
    numPutUtf8(tr);

    for (i = 1; i < h - 1; i++) {
        drawCharAt(r + i, c, vt, NULL, b);
        drawCharAt(r + i, c + w - 1, vt, NULL, b);
    }

    numMoveCursor(r + h - 1, c);
    numPutUtf8(bl);
    for (i = 0; i < w - 2; i++) numPutUtf8(hz);
    numPutUtf8(br);

    printf(RESET_COLOR);
}

/* Draw title: centered, colored text only */
static void drawTitle(int r, int c, int w, const NumberDialog *d) {
    const NumColor *bg = d->hasBg ? &d->bg : &cBg;
    const NumColor *fg = d->hasTitleCol ? &d->titleCol : &cTitle;
    int titleW = displayWidth(d->titleStr);
    int startX = c + (w - titleW) / 2;
    if (startX < c + 1) startX = c + 1;

    numMoveCursor(r, c + 1);
    numBgRgb(bg->r, bg->g, bg->b);
    int i;
    for (i = 0; i < w - 2; i++) putchar(' ');

    numMoveCursor(r, startX);
    numFgRgb(fg->r, fg->g, fg->b);
    printf("%s", d->titleStr);
    printf(RESET_COLOR);
}

/* Draw hint: centered, default foreground */
static void drawHint(int r, int c, int w, const NumberDialog *d) {
    const NumColor *bg = d->hasBg ? &d->bg : &cBg;
    const NumColor *fg = d->hasHintCol ? &d->hintCol : &cHint;
    int hintW = displayWidth(d->hintStr);
    int startX = c + (w - hintW) / 2;
    if (startX < c + 1) startX = c + 1;

    numMoveCursor(r, c + 1);
    numBgRgb(bg->r, bg->g, bg->b);
    int i;
    for (i = 0; i < w - 2; i++) putchar(' ');

    numMoveCursor(r, startX);
    numFgRgb(fg->r, fg->g, fg->b);
    printf("%s", d->hintStr);
    printf(RESET_COLOR);
}

/* Format a number value to string with given decimal places */
static void formatValue(double val, int decimals, char *out, size_t outLen) {
    if (decimals <= 0) {
        snprintf(out, outLen, "%d", (int)val);
    } else {
        char fmt[16];
        snprintf(fmt, sizeof(fmt), "%%.%df", decimals);
        snprintf(out, outLen, fmt, val);
    }
}

/*
 * Draw a single number field row.
 * Layout: [desc]  [-] [value] [+]
 * isSelected: whether this field is currently highlighted.
 * isInputMode: whether we are in input mode for this field.
 */
static void drawFieldRow(int row, int c, int w,
                          const NumberDialog *d, int idx,
                          int isSelected, int isInputMode) {
    const NumberField *f = &d->fields[idx];
    const NumColor *bg        = d->hasBg ? &d->bg : &cBg;
    const NumColor *descFg    = d->hasDescFg ? &d->descFg : &cDescFg;
    const NumColor *valFg     = d->hasValueFg ? &d->valueFg : &cValueFg;
    const NumColor *adjFg     = d->hasAdjustFg ? &d->adjustFg : &cAdjustFg;
    const NumColor *adjBg     = d->hasAdjustBg ? &d->adjustBg : &cAdjustBg;
    const NumColor *adjSelFg  = d->hasAdjustSelFg ? &d->adjustSelFg : &cAdjustSelFg;
    const NumColor *adjSelBg  = d->hasAdjustSelBg ? &d->adjustSelBg : &cAdjustSelBg;
    const NumColor *inBg      = d->hasInputBg ? &d->inputBg : &cInputBg;
    const NumColor *inFg      = d->hasInputFg ? &d->inputFg : &cInputFg;

    char valStr[64];
    if (isInputMode) {
        snprintf(valStr, sizeof(valStr), "%s", gState.inputBuf);
    } else {
        formatValue(f->value, f->decimals, valStr, sizeof(valStr));
    }
    int valW = displayWidth(valStr);
    int descW = displayWidth(f->desc);

    /* Layout: padding + desc + gap + [-] + gap + value + gap + [+] + padding */
    /* [-] and [+] are 3 chars each including brackets */
    int minusW = 3;  /* [-] */
    int plusW = 3;   /* [+] */
    int gap = 1;
    int controlW = minusW + gap + valW + gap + plusW;
    int totalContentW = descW + gap + controlW;
    int padLeft = 2;
    int padRight = w - 2 - totalContentW - padLeft;
    if (padRight < 0) padRight = 0;

    numMoveCursor(row, c + 1);
    numBgRgb(bg->r, bg->g, bg->b);

    int p;
    for (p = 0; p < padLeft; p++) putchar(' ');

    /* Description */
    numFgRgb(descFg->r, descFg->g, descFg->b);
    printf("%s", f->desc);

    /* Gap before controls */
    numBgRgb(bg->r, bg->g, bg->b);
    for (p = 0; p < gap; p++) putchar(' ');

    /* [-] button */
    if (isSelected && !isInputMode) {
        numBgRgb(adjSelBg->r, adjSelBg->g, adjSelBg->b);
        numFgRgb(adjSelFg->r, adjSelFg->g, adjSelFg->b);
    } else {
        numBgRgb(adjBg->r, adjBg->g, adjBg->b);
        numFgRgb(adjFg->r, adjFg->g, adjFg->b);
    }
    putchar('[');
    numPutUtf8(0x2212); /* minus sign */
    putchar(']');

    /* Gap */
    numBgRgb(bg->r, bg->g, bg->b);
    for (p = 0; p < gap; p++) putchar(' ');

    /* Value */
    if (isInputMode) {
        numBgRgb(inBg->r, inBg->g, inBg->b);
        numFgRgb(inFg->r, inFg->g, inFg->b);
    } else if (isSelected) {
        numBgRgb(adjSelBg->r, adjSelBg->g, adjSelBg->b);
        numFgRgb(adjSelFg->r, adjSelFg->g, adjSelFg->b);
    } else {
        numBgRgb(bg->r, bg->g, bg->b);
        numFgRgb(valFg->r, valFg->g, valFg->b);
    }
    printf("%s", valStr);

    /* Gap */
    numBgRgb(bg->r, bg->g, bg->b);
    for (p = 0; p < gap; p++) putchar(' ');

    /* [+] button */
    if (isSelected && !isInputMode) {
        numBgRgb(adjSelBg->r, adjSelBg->g, adjSelBg->b);
        numFgRgb(adjSelFg->r, adjSelFg->g, adjSelFg->b);
    } else {
        numBgRgb(adjBg->r, adjBg->g, adjBg->b);
        numFgRgb(adjFg->r, adjFg->g, adjFg->b);
    }
    putchar('[');
    putchar('+');
    putchar(']');

    /* Right padding fill */
    numBgRgb(bg->r, bg->g, bg->b);
    for (p = 0; p < padRight; p++) putchar(' ');

    printf(RESET_COLOR);
}

/* Draw separator line between content and bottom buttons */
static void drawSeparator(int r, int c, int w, const NumberDialog *d) {
    const NumColor *bg = d->hasBg ? &d->bg : &cBg;
    const NumColor *bdr = d->hasBorder ? &d->border : &cBorder;
    numMoveCursor(r, c + 1);
    numBgRgb(bg->r, bg->g, bg->b);
    numFgRgb(bdr->r, bdr->g, bdr->b);
    int i;
    for (i = 0; i < w - 2; i++) numPutUtf8(0x2500);
    printf(RESET_COLOR);
}

/* Draw bottom button bar */
static void drawBottomBar(int r, int c, int w,
                           const NumberDialog *d) {
    const NumColor *bg        = d->hasBg ? &d->bg : &cBg;
    const NumColor *btmBg     = d->hasBottomBg ? &d->bottomBg : &cBottomBg;
    const NumColor *btmFg     = d->hasBottomFg ? &d->bottomFg : &cBottomFg;
    const NumColor *selBg     = d->hasBottomSelBg ? &d->bottomSelBg : &cBottomSelBg;
    const NumColor *selFg     = d->hasBottomSelFg ? &d->bottomSelFg : &cBottomSelFg;

    int hasLeft = d->hasBottomLeft && d->bottomLeft;
    int hasRight = d->hasBottomRight && d->bottomRight;
    int leftW = hasLeft ? displayWidth(d->bottomLeft) + 4 : 0;
    int rightW = hasRight ? displayWidth(d->bottomRight) + 4 : 0;
    int totalBtnW = leftW + rightW;
    if (hasLeft && hasRight) totalBtnW += 2;

    int startX = c + (w - totalBtnW) / 2;
    if (startX < c + 1) startX = c + 1;

    numMoveCursor(r, c + 1);
    numBgRgb(bg->r, bg->g, bg->b);
    int i;
    for (i = 0; i < w - 2; i++) putchar(' ');

    int currentX = startX;

    if (hasLeft) {
        int isSel = (gState.bottomFocus == 0);
        const NumColor *itemBg = isSel ? selBg : btmBg;
        const NumColor *itemFg = isSel ? selFg : btmFg;

        numMoveCursor(r, currentX);
        numBgRgb(itemBg->r, itemBg->g, itemBg->b);
        numFgRgb(itemFg->r, itemFg->g, itemFg->b);
        putchar(' ');
        putchar('[');
        printf("%s", d->bottomLeft);
        putchar(']');
        putchar(' ');
        printf(RESET_COLOR);
        currentX += leftW;
    }

    if (hasLeft && hasRight) {
        numMoveCursor(r, currentX);
        numBgRgb(bg->r, bg->g, bg->b);
        putchar(' ');
        putchar(' ');
        printf(RESET_COLOR);
        currentX += 2;
    }

    if (hasRight) {
        int isSel = (gState.bottomFocus == 1);
        const NumColor *itemBg = isSel ? selBg : btmBg;
        const NumColor *itemFg = isSel ? selFg : btmFg;

        numMoveCursor(r, currentX);
        numBgRgb(itemBg->r, itemBg->g, itemBg->b);
        numFgRgb(itemFg->r, itemFg->g, itemFg->b);
        putchar(' ');
        putchar('[');
        printf("%s", d->bottomRight);
        putchar(']');
        putchar(' ');
        printf(RESET_COLOR);
    }
}

/*
 * Calculate dialog geometry.
 */
static void numberInitGeometry(const NumberDialog *d) {
    int maxDescW = 0;
    int maxValW = 0;
    int i;
    int titleW = d->titleStr ? displayWidth(d->titleStr) : 0;
    int hintW = d->hintStr ? displayWidth(d->hintStr) : 0;

    for (i = 0; i < d->fieldCount; i++) {
        int dw = displayWidth(d->fields[i].desc);
        if (dw > maxDescW) maxDescW = dw;
        char valStr[64];
        formatValue(d->fields[i].maxVal, d->fields[i].decimals, valStr, sizeof(valStr));
        int vw = displayWidth(valStr);
        if (vw > maxValW) maxValW = vw;
    }

    /* Each row: padding(2) + desc + gap(1) + [-](3) + gap(1) + value + gap(1) + [+](3) + padding(2) */
    int controlW = 3 + 1 + maxValW + 1 + 3; /* [-] gap value gap [+] */
    int contentW = maxDescW + 1 + controlW + 4; /* desc + gap + controls + padding */

    int bottomW = 0;
    if (d->hasBottomLeft && d->bottomLeft)
        bottomW += displayWidth(d->bottomLeft) + 4;
    if (d->hasBottomRight && d->bottomRight)
        bottomW += displayWidth(d->bottomRight) + 4;
    if (d->hasBottomLeft && d->hasBottomRight) bottomW += 2;

    int totalContentW = contentW;
    if (titleW + 4 > totalContentW) totalContentW = titleW + 4;
    if (hintW + 4 > totalContentW) totalContentW = hintW + 4;
    if (bottomW > totalContentW) totalContentW = bottomW;

    gState.dlgW = d->w;
    if (gState.dlgW == 0) {
        gState.dlgW = totalContentW;
        if (gState.dlgW < 24) gState.dlgW = 24;
        if (gState.dlgW > gState.termW - 4)
            gState.dlgW = gState.termW - 4;
    }

    int hasBottom = (d->hasBottomLeft || d->hasBottomRight);
    int minContentH = 1;
    int maxContentH = gState.termH - 8;
    if (hasBottom) maxContentH -= 2;
    int contentH = d->fieldCount;
    if (contentH < minContentH) contentH = minContentH;
    if (contentH > maxContentH) contentH = maxContentH;

    /* height = border(1) + title(1) + hint(1) + gap(1) + content + gap(1) + separator(1) + bottom(1) + border(1) */
    gState.dlgH = 1 + 1 + 1 + 1 + contentH + 1 + (hasBottom ? 2 : 0) + 1;
    if (gState.dlgH > gState.termH - 2)
        gState.dlgH = gState.termH - 2;

    gState.dlgX = d->x;
    gState.dlgY = d->y;
    if (gState.dlgX == 0) gState.dlgX = (gState.termW - gState.dlgW) / 2;
    if (gState.dlgY == 0) gState.dlgY = (gState.termH - gState.dlgH) / 2;
    if (gState.dlgX < 1) gState.dlgX = 1;
    if (gState.dlgY < 1) gState.dlgY = 1;

    gState.selIdx = d->initialSel;
    if (gState.selIdx < 0) gState.selIdx = 0;
    if (gState.selIdx >= d->fieldCount) gState.selIdx = 0;

    if (d->hasBottomLeft) {
        gState.bottomFocus = 0;
    } else if (d->hasBottomRight) {
        gState.bottomFocus = 1;
    } else {
        gState.bottomFocus = -1;
    }

    gState.scrollOffset = 0;
    gState.inInputMode = 0;
    gState.inputLen = 0;
    gState.inputBuf[0] = '\0';
}

/*
 * Full redraw.
 */
static void drawNumberFull(const NumberDialog *d) {
    int r = gState.dlgY;
    int c = gState.dlgX;
    int w = gState.dlgW;
    int h = gState.dlgH;
    const NumColor *bg = d->hasBg ? &d->bg : &cBg;
    int hasBottom = (d->hasBottomLeft || d->hasBottomRight);
    int contentH = h - (hasBottom ? 7 : 5);
    int i;

    if (d->bgDrawFn) {
        d->bgDrawFn(gState.termW, gState.termH, d->bgDrawData);
    } else {
        fillRect(r, c, h, w, bg);
    }

    if (d->shadowEnabled) {
        const NumColor *sh = d->hasShadow ? &d->shadow : &cShadow;
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

    /* Fill gap row between hint and fields */
    numMoveCursor(r + 3, c + 1);
    numBgRgb(bg->r, bg->g, bg->b);
    for (i = 0; i < w - 2; i++) putchar(' ');
    printf(RESET_COLOR);

    int fieldStartRow = r + 4;
    for (i = 0; i < contentH; i++) {
        int fieldIdx = gState.scrollOffset + i;
        if (fieldIdx < d->fieldCount) {
            drawFieldRow(fieldStartRow + i, c, w, d, fieldIdx,
                          fieldIdx == gState.selIdx,
                          gState.inInputMode && fieldIdx == gState.selIdx);
        } else {
            numMoveCursor(fieldStartRow + i, c + 1);
            numBgRgb(bg->r, bg->g, bg->b);
            int p;
            for (p = 0; p < w - 2; p++) putchar(' ');
            printf(RESET_COLOR);
        }
    }

    /* Scroll indicators */
    if (gState.scrollOffset > 0) {
        numMoveCursor(fieldStartRow, c + w - 2);
        numBgRgb(bg->r, bg->g, bg->b);
        numFgRgb(cBorder.r, cBorder.g, cBorder.b);
        numPutUtf8(0x25B2);
        printf(RESET_COLOR);
    } else {
        numMoveCursor(fieldStartRow, c + w - 2);
        numBgRgb(bg->r, bg->g, bg->b);
        putchar(' ');
        printf(RESET_COLOR);
    }
    if (gState.scrollOffset + contentH < d->fieldCount) {
        numMoveCursor(fieldStartRow + contentH - 1, c + w - 2);
        numBgRgb(bg->r, bg->g, bg->b);
        numFgRgb(cBorder.r, cBorder.g, cBorder.b);
        numPutUtf8(0x25BC);
        printf(RESET_COLOR);
    } else {
        numMoveCursor(fieldStartRow + contentH - 1, c + w - 2);
        numBgRgb(bg->r, bg->g, bg->b);
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

/* Redraw only the content area (fields) */
static void drawNumberContent(const NumberDialog *d) {
    int r = gState.dlgY;
    int c = gState.dlgX;
    int w = gState.dlgW;
    int h = gState.dlgH;
    const NumColor *bg = d->hasBg ? &d->bg : &cBg;
    int hasBottom = (d->hasBottomLeft || d->hasBottomRight);
    int contentH = h - (hasBottom ? 7 : 5);
    /* Fill gap row between hint and fields */
    numMoveCursor(r + 3, c + 1);
    numBgRgb(bg->r, bg->g, bg->b);
    int j;
    for (j = 0; j < w - 2; j++) putchar(' ');
    printf(RESET_COLOR);

    int fieldStartRow = r + 4;
    int i;

    for (i = 0; i < contentH; i++) {
        int fieldIdx = gState.scrollOffset + i;
        if (fieldIdx < d->fieldCount) {
            drawFieldRow(fieldStartRow + i, c, w, d, fieldIdx,
                          fieldIdx == gState.selIdx,
                          gState.inInputMode && fieldIdx == gState.selIdx);
        } else {
            numMoveCursor(fieldStartRow + i, c + 1);
            numBgRgb(bg->r, bg->g, bg->b);
            int p;
            for (p = 0; p < w - 2; p++) putchar(' ');
            printf(RESET_COLOR);
        }
    }

    if (gState.scrollOffset > 0) {
        numMoveCursor(fieldStartRow, c + w - 2);
        numBgRgb(bg->r, bg->g, bg->b);
        numFgRgb(cBorder.r, cBorder.g, cBorder.b);
        numPutUtf8(0x25B2);
        printf(RESET_COLOR);
    } else {
        numMoveCursor(fieldStartRow, c + w - 2);
        numBgRgb(bg->r, bg->g, bg->b);
        putchar(' ');
        printf(RESET_COLOR);
    }
    if (gState.scrollOffset + contentH < d->fieldCount) {
        numMoveCursor(fieldStartRow + contentH - 1, c + w - 2);
        numBgRgb(bg->r, bg->g, bg->b);
        numFgRgb(cBorder.r, cBorder.g, cBorder.b);
        numPutUtf8(0x25BC);
        printf(RESET_COLOR);
    } else {
        numMoveCursor(fieldStartRow + contentH - 1, c + w - 2);
        numBgRgb(bg->r, bg->g, bg->b);
        putchar(' ');
        printf(RESET_COLOR);
    }

    fflush(stdout);
}

/* Redraw only a single field row */
static void drawFieldAt(const NumberDialog *d, int idx) {
    int r = gState.dlgY;
    int c = gState.dlgX;
    int w = gState.dlgW;
    int h = gState.dlgH;
    int hasBottom = (d->hasBottomLeft || d->hasBottomRight);
    int contentH = h - (hasBottom ? 7 : 5);
    int fieldStartRow = r + 4;

    int rowInView = idx - gState.scrollOffset;
    if (rowInView < 0 || rowInView >= contentH) return;

    drawFieldRow(fieldStartRow + rowInView, c, w, d, idx,
                  idx == gState.selIdx,
                  gState.inInputMode && idx == gState.selIdx);
    fflush(stdout);
}

/* Redraw bottom bar only */
static void drawBottomBarOnly(const NumberDialog *d) {
    int r = gState.dlgY;
    int c = gState.dlgX;
    int w = gState.dlgW;
    int h = gState.dlgH;
    int btmRow = r + h - 2;
    drawBottomBar(btmRow, c, w, d);
    fflush(stdout);
}

/* Ensure selected item is in view */
static void numberEnsureVisible(const NumberDialog *d) {
    int hasBottom = (d->hasBottomLeft || d->hasBottomRight);
    int contentH = gState.dlgH - (hasBottom ? 7 : 5);

    if (gState.selIdx < gState.scrollOffset) {
        gState.scrollOffset = gState.selIdx;
    } else if (gState.selIdx >= gState.scrollOffset + contentH) {
        gState.scrollOffset = gState.selIdx - contentH + 1;
    }
}

/* Clamp value to [min, max] */
static double clampValue(double val, double minVal, double maxVal) {
    if (val < minVal) return minVal;
    if (val > maxVal) return maxVal;
    return val;
}

/* Round to N decimal places */
static double roundDecimals(double val, int decimals) {
    if (decimals <= 0) return round(val);
    double factor = pow(10.0, decimals);
    return round(val * factor) / factor;
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
#define KEY_BACKSPACE 9
#define KEY_DELETE   10

typedef struct {
    int type;
    int ch;              /* raw char for input mode */
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
    InputEvent ev = {KEY_NONE, 0, 0, 0, 0, 0};
    int c = readByte();
    if (c < 0) return ev;
    ev.ch = c;

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
            if (c3 == '3') {
                int c4 = readByte();
                if (c4 == '~') { ev.type = KEY_DELETE; return ev; }
            }

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
    if (c == 127 || c == 8) { ev.type = KEY_BACKSPACE; return ev; }

    if (c == 'w' || c == 'W' || c == 'k' || c == 'K') { ev.type = KEY_UP; return ev; }
    if (c == 's' || c == 'S' || c == 'j' || c == 'J') { ev.type = KEY_DOWN; return ev; }
    if (c == 'a' || c == 'A' || c == 'h' || c == 'H') { ev.type = KEY_LEFT; return ev; }
    if (c == 'd' || c == 'D' || c == 'l' || c == 'L') { ev.type = KEY_RIGHT; return ev; }

    if (c == 10 || c == 13) { ev.type = KEY_ENTER; return ev; }

    if (c == 3 || c == 4 || c == 'q' || c == 'Q') { ev.type = KEY_ESC; return ev; }

    return ev;
}

/* Hit test for fields */
static int hitTestField(int mx, int my, const NumberDialog *d) {
    int r = gState.dlgY;
    int c = gState.dlgX;
    int h = gState.dlgH;
    int hasBottom = (d->hasBottomLeft || d->hasBottomRight);
    int contentH = h - (hasBottom ? 7 : 5);
    int fieldStartRow = r + 4;

    if (mx < c + 1 || mx >= c + gState.dlgW - 1) return -1;
    if (my < fieldStartRow || my >= fieldStartRow + contentH) return -1;

    int idx = gState.scrollOffset + (my - fieldStartRow);
    if (idx < 0 || idx >= d->fieldCount) return -1;
    return idx;
}

/* Hit test for bottom buttons */
static int hitTestBottomItem(int mx, int my, const NumberDialog *d) {
    int r = gState.dlgY;
    int c = gState.dlgX;
    int w = gState.dlgW;
    int h = gState.dlgH;
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

/* Hit test for [-] or [+] within a field row */
/* Returns: -1=none, 0=minus, 1=plus */
static int hitTestAdjust(int mx, int my, const NumberDialog *d, int fieldIdx) {
    int r = gState.dlgY;
    int c = gState.dlgX;
    int w = gState.dlgW;
    int fieldStartRow = r + 4;

    if (my != fieldStartRow + (fieldIdx - gState.scrollOffset)) return -1;
    if (mx < c + 1 || mx >= c + w - 1) return -1;

    const NumberField *f = &d->fields[fieldIdx];
    int descW = displayWidth(f->desc);
    char valStr[64];
    formatValue(f->value, f->decimals, valStr, sizeof(valStr));
    int valW = displayWidth(valStr);

    int minusW = 3;
    int plusW = 3;
    int gap = 1;
    int padLeft = 2;

    int controlStartX = c + 1 + padLeft + descW + gap;
    int minusEndX = controlStartX + minusW;
    int valStartX = minusEndX + gap;
    int valEndX = valStartX + valW;
    int plusStartX = valEndX + gap;
    int plusEndX = plusStartX + plusW;

    if (mx >= controlStartX && mx < minusEndX) return 0;
    if (mx >= plusStartX && mx < plusEndX) return 1;
    return -1;
}

/* ================================================================== */
/* Public API                                                         */
/* ================================================================== */

NumberDialog* numberCreate(void) {
    NumberDialog *d = calloc(1, sizeof(NumberDialog));
    if (!d) return NULL;
    d->fieldCap = 8;
    d->fields = calloc(d->fieldCap, sizeof(NumberField));
    if (!d->fields) { free(d); return NULL; }
    d->shadowEnabled = 1;
    d->borderDouble = 0;
    d->roundedCorners = 1;
    d->useAltBuffer = 1;
    d->useMouse = 1;
    d->initialSel = 0;
    return d;
}

void numberFree(NumberDialog *d) {
    int i;
    if (!d) return;
    free(d->titleStr);
    free(d->hintStr);
    for (i = 0; i < d->fieldCount; i++) {
        free(d->fields[i].desc);
    }
    free(d->fields);
    free(d->bottomLeft);
    free(d->bottomRight);
    free(d);
}

void numberResultFree(NumberResult *res) {
    if (!res) return;
    free(res->values);
    res->values = NULL;
    res->fieldCount = 0;
}

void numberTitle(NumberDialog *d, const char *title) {
    if (!d) return;
    free(d->titleStr);
    d->titleStr = title ? strdup(title) : NULL;
}

void numberHint(NumberDialog *d, const char *hint) {
    if (!d) return;
    free(d->hintStr);
    d->hintStr = hint ? strdup(hint) : NULL;
}

int numberField(NumberDialog *d, const char *desc,
                double minVal, double maxVal,
                double defaultVal, int decimals) {
    NumberField *f;
    if (!d || !desc) return -1;
    if (d->fieldCount >= d->fieldCap) {
        int newCap = d->fieldCap * 2;
        NumberField *newFields = realloc(d->fields, newCap * sizeof(NumberField));
        if (!newFields) return -1;
        memset(newFields + d->fieldCap, 0, (newCap - d->fieldCap) * sizeof(NumberField));
        d->fields = newFields;
        d->fieldCap = newCap;
    }
    f = &d->fields[d->fieldCount];
    f->desc = strdup(desc);
    f->minVal = minVal;
    f->maxVal = maxVal;
    f->value = clampValue(defaultVal, minVal, maxVal);
    f->decimals = decimals;
    f->step = (decimals <= 0) ? 1.0 : (1.0 / pow(10.0, decimals));
    if (f->step < 0.001) f->step = 0.001;
    return d->fieldCount++;
}

void numberBottomLeft(NumberDialog *d, const char *label) {
    if (!d) return;
    free(d->bottomLeft);
    d->bottomLeft = label ? strdup(label) : NULL;
    d->hasBottomLeft = (label != NULL);
}

void numberBottomRight(NumberDialog *d, const char *label) {
    if (!d) return;
    free(d->bottomRight);
    d->bottomRight = label ? strdup(label) : NULL;
    d->hasBottomRight = (label != NULL);
}

void numberColor(NumberDialog *d, int which, NumColor c) {
    if (!d) return;
    switch (which) {
        case NUMBER_COLOR_BG:            d->bg = c; d->hasBg = 1; break;
        case NUMBER_COLOR_FG:              d->fg = c; d->hasFg = 1; break;
        case NUMBER_COLOR_BORDER:          d->border = c; d->hasBorder = 1; break;
        case NUMBER_COLOR_SHADOW:          d->shadow = c; d->hasShadow = 1; break;
        case NUMBER_COLOR_TITLE:           d->titleCol = c; d->hasTitleCol = 1; break;
        case NUMBER_COLOR_HINT:            d->hintCol = c; d->hasHintCol = 1; break;
        case NUMBER_COLOR_DESC_FG:         d->descFg = c; d->hasDescFg = 1; break;
        case NUMBER_COLOR_VALUE_FG:        d->valueFg = c; d->hasValueFg = 1; break;
        case NUMBER_COLOR_ADJUST_FG:       d->adjustFg = c; d->hasAdjustFg = 1; break;
        case NUMBER_COLOR_ADJUST_BG:       d->adjustBg = c; d->hasAdjustBg = 1; break;
        case NUMBER_COLOR_ADJUST_SEL_FG:   d->adjustSelFg = c; d->hasAdjustSelFg = 1; break;
        case NUMBER_COLOR_ADJUST_SEL_BG:   d->adjustSelBg = c; d->hasAdjustSelBg = 1; break;
        case NUMBER_COLOR_BOTTOM_FG:       d->bottomFg = c; d->hasBottomFg = 1; break;
        case NUMBER_COLOR_BOTTOM_BG:       d->bottomBg = c; d->hasBottomBg = 1; break;
        case NUMBER_COLOR_BOTTOM_SEL_FG:   d->bottomSelFg = c; d->hasBottomSelFg = 1; break;
        case NUMBER_COLOR_BOTTOM_SEL_BG:   d->bottomSelBg = c; d->hasBottomSelBg = 1; break;
        case NUMBER_COLOR_INPUT_BG:        d->inputBg = c; d->hasInputBg = 1; break;
        case NUMBER_COLOR_INPUT_FG:        d->inputFg = c; d->hasInputFg = 1; break;
    }
}

void numberStyle(NumberDialog *d, int shadow, int doubleBorder, int rounded) {
    if (!d) return;
    d->shadowEnabled = shadow;
    d->borderDouble = doubleBorder;
    d->roundedCorners = rounded;
}

void numberPos(NumberDialog *d, int x, int y, int w, int h) {
    if (!d) return;
    d->x = x; d->y = y; d->w = w; d->h = h;
}

void numberInitial(NumberDialog *d, int idx) {
    if (!d) return;
    d->initialSel = idx;
}

void numberStep(NumberDialog *d, int fieldIdx, double step) {
    if (!d || fieldIdx < 0 || fieldIdx >= d->fieldCount) return;
    d->fields[fieldIdx].step = step;
}

void numberUseAltBuffer(NumberDialog *d, int use) {
    if (!d) return;
    d->useAltBuffer = use;
}

void numberUseMouse(NumberDialog *d, int use) {
    if (!d) return;
    d->useMouse = use;
}

void numberBgDraw(NumberDialog *d, NumberBgDrawFn fn, void *userData) {
    if (!d) return;
    d->bgDrawFn = fn;
    d->bgDrawData = userData;
}

/* ================================================================== */
/* numberRun                                                          */
/* ================================================================== */

NumberResult numberRun(NumberDialog *d) {
    NumberResult res = {NULL, 0, 0, -1};
    InputEvent ev;
    struct sigaction sa, oldSa;
    int hasBottom;
    int i;

    if (!d || d->fieldCount == 0) return res;

    hasBottom = (d->hasBottomLeft || d->hasBottomRight);

    memset(&gState, 0, sizeof(gState));
    getTermSize();
    numberInitGeometry(d);

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
    drawNumberFull(d);

    while (gState.running) {
        if (gResized) {
            gResized = 0;
            getTermSize();
            numberInitGeometry(d);
            printf(CLEAR_SCREEN);
            drawNumberFull(d);
        }

        ev = parseInput();

        /* Input mode: handle typing */
        if (gState.inInputMode) {
            if (ev.type == KEY_ENTER) {
                /* Confirm input */
                gState.inputBuf[gState.inputLen] = '\0';
                double newVal = strtod(gState.inputBuf, NULL);
                NumberField *f = &d->fields[gState.selIdx];
                f->value = clampValue(roundDecimals(newVal, f->decimals), f->minVal, f->maxVal);
                gState.inInputMode = 0;
                gState.inputLen = 0;
                gState.inputBuf[0] = '\0';
                drawFieldAt(d, gState.selIdx);
            } else if (ev.type == KEY_ESC) {
                /* Cancel input */
                gState.inInputMode = 0;
                gState.inputLen = 0;
                gState.inputBuf[0] = '\0';
                drawFieldAt(d, gState.selIdx);
            } else if (ev.type == KEY_BACKSPACE) {
                if (gState.inputLen > 0) {
                    gState.inputLen--;
                    gState.inputBuf[gState.inputLen] = '\0';
                    drawFieldAt(d, gState.selIdx);
                }
            } else if (ev.ch >= 32 && ev.ch < 127) {
                /* Accept digits, decimal point, minus sign */
                char ch = (char)ev.ch;
                int isDigit = (ch >= '0' && ch <= '9');
                int isDot = (ch == '.');
                int isMinus = (ch == '-');
                int hasDot = (strchr(gState.inputBuf, '.') != NULL);

                if (isDigit || (isDot && !hasDot) || (isMinus && gState.inputLen == 0)) {
                    if (gState.inputLen < 63) {
                        gState.inputBuf[gState.inputLen++] = ch;
                        gState.inputBuf[gState.inputLen] = '\0';
                        drawFieldAt(d, gState.selIdx);
                    }
                }
            }
            continue;
        }

        switch (ev.type) {
            case KEY_UP:
                if (d->fieldCount > 0) {
                    if (gState.selIdx > 0) {
                        gState.selIdx--;
                    } else {
                        gState.selIdx = d->fieldCount - 1;
                    }
                    numberEnsureVisible(d);
                    drawNumberContent(d);
                }
                break;

            case KEY_DOWN:
                if (d->fieldCount > 0) {
                    if (gState.selIdx < d->fieldCount - 1) {
                        gState.selIdx++;
                    } else {
                        gState.selIdx = 0;
                    }
                    numberEnsureVisible(d);
                    drawNumberContent(d);
                }
                break;

            case KEY_LEFT:
                if (d->fieldCount > 0) {
                    NumberField *f = &d->fields[gState.selIdx];
                    double newVal = f->value - f->step;
                    f->value = clampValue(roundDecimals(newVal, f->decimals), f->minVal, f->maxVal);
                    drawFieldAt(d, gState.selIdx);
                }
                break;

            case KEY_RIGHT:
                if (d->fieldCount > 0) {
                    NumberField *f = &d->fields[gState.selIdx];
                    double newVal = f->value + f->step;
                    f->value = clampValue(roundDecimals(newVal, f->decimals), f->minVal, f->maxVal);
                    drawFieldAt(d, gState.selIdx);
                }
                break;

            case KEY_ENTER:
                /* Enter input mode */
                if (d->fieldCount > 0) {
                    gState.inInputMode = 1;
                    gState.inputLen = 0;
                    gState.inputBuf[0] = '\0';
                    drawFieldAt(d, gState.selIdx);
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

            case KEY_ESC:
                gState.cancelled = 1;
                gState.running = 0;
                break;

            case KEY_MOUSE:
                if (ev.mouseBtn == 0 || ev.mouseBtn == 2) {
                    int fieldIdx = hitTestField(ev.mouseX, ev.mouseY, d);
                    if (fieldIdx >= 0) {
                        gState.selIdx = fieldIdx;
                        numberEnsureVisible(d);

                        int adj = hitTestAdjust(ev.mouseX, ev.mouseY, d, fieldIdx);
                        if (adj == 0) {
                            /* Clicked [-] */
                            NumberField *f = &d->fields[fieldIdx];
                            double newVal = f->value - f->step;
                            f->value =clampValue(roundDecimals(newVal, f->decimals), f->minVal, f->maxVal);
                            drawNumberContent(d);

                            usleep(40 * 1000);
                            tcflush(STDIN_FILENO, TCIFLUSH);
                        } else if (adj == 1) {
                            /* Clicked [+] */
                            NumberField *f = &d->fields[fieldIdx];
                            double newVal = f->value + f->step;
                            f->value = clampValue(roundDecimals(newVal, f->decimals), f->minVal, f->maxVal);
                            drawNumberContent(d);

                            usleep(40 * 1000);
                            tcflush(STDIN_FILENO, TCIFLUSH);
                        } else {
                            drawNumberContent(d);
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

    /* Build result */
    res.values = malloc(d->fieldCount * sizeof(double));
    if (res.values) {
        for (i = 0; i < d->fieldCount; i++) {
            res.values[i] = d->fields[i].value;
        }
        res.fieldCount = d->fieldCount;
    }
    res.confirmed = gState.confirmed;
    if (!gState.confirmed && !gState.cancelled) {
        res.confirmed = 0;
    }
    return res;
}
