#ifndef DECLARATIVE_BROWSER_H
#define DECLARATIVE_BROWSER_H

/* declarative_browser.h
 * Production-grade, dependency-free C TUI text browser.
 * Background: RGB(255, 250, 240) warm ivory.
 *
 * Link with declarative_browser.c
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Color enum for browserColor() */
enum {
    BROWSER_COLOR_BG,
    BROWSER_COLOR_FG,
    BROWSER_COLOR_HINT_BG,
    BROWSER_COLOR_HINT_FG,
    BROWSER_COLOR_PROMPT_FG,
    BROWSER_COLOR_BORDER,
    BROWSER_COLOR_BUTTON_BG,
    BROWSER_COLOR_BUTTON_FG,
    BROWSER_COLOR_BUTTON_HL_BG,
    BROWSER_COLOR_BUTTON_HL_FG
};

/* Result returned by browserRun() */
typedef struct {
    int selectedButton; /* 0 = left, 1 = right, -1 = cancelled */
} BrowserResult;

/* Browser handle: opaque, allocated via browserCreate() */
typedef struct TextBrowser TextBrowser;

/* Create a new empty browser with default settings */
TextBrowser* browserCreate(void);

/* Free all memory allocated by the browser */
void browserFree(TextBrowser *b);

/* Set the title/hint text shown at the top (blue bar, only text area colored). Pass NULL to clear. */
void browserHint(TextBrowser *b, const char *hint);

/* Set a short prompt text shown above the content box. Pass NULL to clear. */
void browserPrompt(TextBrowser *b, const char *prompt);

/* Set the main body text. Supports multi-line with \n. Pass NULL to clear. */
void browserText(TextBrowser *b, const char *text);

/* Set left button text. Pass NULL to hide. */
void browserButtonLeft(TextBrowser *b, const char *label);

/* Set right button text. Pass NULL to hide. */
void browserButtonRight(TextBrowser *b, const char *label);

/* Set a color by enum */
void browserColor(TextBrowser *b, int which, Color c);

/* Set visual style: border, rounded corners */
void browserStyle(TextBrowser *b, int border, int rounded);

/* Set position and size. Use 0 for auto-center / auto-size. */
void browserPos(TextBrowser *b, int x, int y, int w, int h);

/* Set whether to use alternate screen buffer (default: 1) */
void browserUseAltBuffer(TextBrowser *b, int use);

/* Set whether to enable mouse tracking (default: 1) */
void browserUseMouse(TextBrowser *b, int use);

/* Run the interactive browser. Returns selected button index, or -1 on cancel. */
BrowserResult browserRun(TextBrowser *b);

#ifdef __cplusplus
}
#endif

#endif
