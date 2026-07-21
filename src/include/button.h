#ifndef DECLARATIVE_BUTTON_H
#define DECLARATIVE_BUTTON_H

/* declarative_button.h
 * Public API for dependency-free C TUI button button.
 * Link with declarative_button.c
 *
 * Features:
 *   - Title (blue text, text-only colored)
 *   - Hint (short prompt below title)
 *   - Vertical button list (custom count, scrollable)
 *   - Bottom dual buttons (left/right, individually optional)
 *   - When bottom enabled: top buttons = select only, bottom = confirm/cancel
 *   - When bottom disabled: top buttons = direct confirm
 *   - Tab key switches bottom button focus
 *   - Full border, warm ivory background
 *   - Wide character (CJK) support
 *   - Zero dependencies, pure ANSI
 */

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "menu.h"

/* ANSI helper functions exposed for custom background callbacks */
void moveCursor(int row, int col);
void bgRgb(int r, int g, int b);
void fgRgb(int r, int g, int b);
void putUtf8(int codepoint);

/* Color enum for buttonColor() */
enum {
    BUTTON_COLOR_BG,
    BUTTON_COLOR_FG,
    BUTTON_COLOR_BORDER,
    BUTTON_COLOR_SHADOW,
    BUTTON_COLOR_TITLE,
    BUTTON_COLOR_HINT,
    BUTTON_COLOR_BTN_FG,
    BUTTON_COLOR_BTN_BG,
    BUTTON_COLOR_BTN_SEL_FG,
    BUTTON_COLOR_BTN_SEL_BG,
    BUTTON_COLOR_BOTTOM_FG,
    BUTTON_COLOR_BOTTOM_BG,
    BUTTON_COLOR_BOTTOM_SEL_FG,
    BUTTON_COLOR_BOTTOM_SEL_BG
};

/* Button handle: opaque, allocated via buttonCreate() */
typedef struct Button Button;

/* Result returned by buttonRun() */
typedef struct {
    int selectedTop;     /* selected top button index, -1 if none */
    int confirmed;       /* 1 = confirmed (OK/Enter), 0 = cancelled (Esc/Cancel) */
    int bottomButton;    /* which bottom button was pressed: 0=left, 1=right, -1=none */
} ButtonResult;

/* Create a new empty button with default settings */
Button* buttonCreate(void);

/* Free all memory allocated by the button */
void buttonFree(Button *d);

/* Set button title (blue colored, text-only) */
void buttonTitle(Button *d, const char *title);

/* Set hint text below title */
void buttonHint(Button *d, const char *hint);

/* Add a top button (vertical list). Returns its index. */
int buttonAdd(Button *b, const char *label);

/* Set bottom left button text. Pass NULL to disable. */
void buttonBottomLeft(Button *d, const char *label);

/* Set bottom right button text. Pass NULL to disable. */
void buttonBottomRight(Button *d, const char *label);

/* Set a color by enum */
void buttonColor(Button *d, int which, Color c);

/* Set visual style: shadow, double border, rounded corners */
void buttonStyle(Button *d, int shadow, int doubleBorder, int rounded);

/* Set position and size. Use 0 for auto-center / auto-size. */
void buttonPos(Button *d, int x, int y, int w, int h);

/* Set initially selected top button index */
void buttonInitial(Button *d, int idx);

/* Set whether to use alternate screen buffer (default: 1) */
void buttonUseAltBuffer(Button *d, int use);

/* Set whether to enable mouse tracking (default: 1) */
void buttonUseMouse(Button *d, int use);

/* Background draw callback: user provides a function to paint the whole screen.
 * Called once on init and once on each resize. Pass NULL to disable. */
typedef void (*ButtonBgDrawFn)(int termW, int termH, void *userData);
void buttonBgDraw(Button *d, ButtonBgDrawFn fn, void *userData);

/* Run the interactive button. Returns result. */
ButtonResult buttonRun(Button *d);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // DECLARATIVE_BUTTON_H
