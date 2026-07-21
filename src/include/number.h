#ifndef DECLARATIVE_NUMBER_H
#define DECLARATIVE_NUMBER_H

/* declarative_number.h
 * Public API for dependency-free C TUI number dialog.
 * Link with declarative_number.c
 *
 * Features:
 *   - Title (blue text, text-only colored)
 *   - Hint (short prompt below title)
 *   - Vertical number field list: description + [-] value [+]
 *   - Each field: min, max, default, decimal places (0 = integer)
 *   - Up/Down navigate fields, Left/Right adjust value
 *   - Enter to enter input mode (type number), Enter again to confirm
 *   - Bottom dual buttons (left/right, individually optional, at least one required)
 *   - Full border, warm ivory background
 *   - Wide character (CJK) support
 *   - Zero dependencies, pure ANSI
 */

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/* ANSI helper functions exposed for custom background callbacks */
void numMoveCursor(int row, int col);
void numBgRgb(int r, int g, int b);
void numFgRgb(int r, int g, int b);
void numPutUtf8(int codepoint);

/* RGB color */
typedef struct {
    unsigned char r, g, b;
} NumColor;

#define NC(r,g,b) ((NumColor){r,g,b})

/* Color enum for numberColor() */
enum {
    NUMBER_COLOR_BG,
    NUMBER_COLOR_FG,
    NUMBER_COLOR_BORDER,
    NUMBER_COLOR_SHADOW,
    NUMBER_COLOR_TITLE,
    NUMBER_COLOR_HINT,
    NUMBER_COLOR_DESC_FG,
    NUMBER_COLOR_VALUE_FG,
    NUMBER_COLOR_ADJUST_FG,
    NUMBER_COLOR_ADJUST_BG,
    NUMBER_COLOR_ADJUST_SEL_FG,
    NUMBER_COLOR_ADJUST_SEL_BG,
    NUMBER_COLOR_BOTTOM_FG,
    NUMBER_COLOR_BOTTOM_BG,
    NUMBER_COLOR_BOTTOM_SEL_FG,
    NUMBER_COLOR_BOTTOM_SEL_BG,
    NUMBER_COLOR_INPUT_BG,
    NUMBER_COLOR_INPUT_FG
};

/* Number field handle: opaque */
typedef struct NumberField NumberField;

/* Number dialog handle: opaque, allocated via numberCreate() */
typedef struct NumberDialog NumberDialog;

/* Result returned by numberRun() */
typedef struct {
    double *values;      /* array of final values, caller must free */
    int fieldCount;      /* number of fields */
    int confirmed;       /* 1 = confirmed (OK), 0 = cancelled (Esc/Cancel) */
    int bottomButton;    /* which bottom button: 0=left, 1=right, -1=none */
} NumberResult;

/* Create a new empty number dialog with default settings */
NumberDialog* numberCreate(void);

/* Free all memory allocated by the dialog and result values */
void numberFree(NumberDialog *d);
void numberResultFree(NumberResult *res);

/* Set dialog title (blue colored, text-only) */
void numberTitle(NumberDialog *d, const char *title);

/* Set hint text below title */
void numberHint(NumberDialog *d, const char *hint);

/*
 * Add a number field.
 *   desc: short description shown on the left
 *   minVal, maxVal: range limits (inclusive)
 *   defaultVal: initial value
 *   decimals: number of decimal places (0 = integer)
 * Returns field index, or -1 on error.
 */
int numberField(NumberDialog *d, const char *desc,
                double minVal, double maxVal,
                double defaultVal, int decimals);

/* Set bottom left button text. Pass NULL to disable. */
void numberBottomLeft(NumberDialog *d, const char *label);

/* Set bottom right button text. Pass NULL to disable. */
void numberBottomRight(NumberDialog *d, const char *label);

/* Set a color by enum */
void numberColor(NumberDialog *d, int which, NumColor c);

/* Set visual style: shadow, double border, rounded corners */
void numberStyle(NumberDialog *d, int shadow, int doubleBorder, int rounded);

/* Set position and size. Use 0 for auto-center / auto-size. */
void numberPos(NumberDialog *d, int x, int y, int w, int h);

/* Set initially selected field index */
void numberInitial(NumberDialog *d, int idx);

/* Set step size for +/- adjustment (default: 1.0 for ints, 0.1 for decimals) */
void numberStep(NumberDialog *d, int fieldIdx, double step);

/* Set whether to use alternate screen buffer (default: 1) */
void numberUseAltBuffer(NumberDialog *d, int use);

/* Set whether to enable mouse tracking (default: 1) */
void numberUseMouse(NumberDialog *d, int use);

/* Background draw callback: user provides a function to paint the whole screen.
 * Called once on init and once on each resize. Pass NULL to disable. */
typedef void (*NumberBgDrawFn)(int termW, int termH, void *userData);
void numberBgDraw(NumberDialog *d, NumberBgDrawFn fn, void *userData);

/* Run the interactive number dialog. Returns result. Caller must free values via numberResultFree(). */
NumberResult numberRun(NumberDialog *d);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // DECLARATIVE_NUMBER_H
