#ifndef DECLARATIVE_MENU_H
#define DECLARATIVE_MENU_H

/* declarative_menu.h
 * Public API for dependency-free C TUI menu.
 * Link with declarative_menu.c
 */

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/* RGB color */
typedef struct {
    unsigned char r, g, b;
} Color;

#define C(r,g,b) ((Color){r,g,b})

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

/* Menu handle: opaque, allocated via menuCreate() */
typedef struct Menu Menu;

/* Result returned by menuRun() */
typedef struct {
    int selectedTab;
    int selectedItem;
} MenuResult;

/* Create a new empty menu with default settings */
Menu* menuCreate(void);

/* Free all memory allocated by the menu */
void menuFree(Menu *m);

/* Add a new tab, return its index. Auto-resizes. */
int menuTab(Menu *m, const char *title);

/* Add an item to the specified tab. Auto-resizes. */
int menuItem(Menu *m, int tabIdx, const char *label, int icon);

/* Set a color by enum: MENU_COLOR_BG, MENU_COLOR_FG, etc. */
void menuColor(Menu *m, int which, Color c);

/* Set visual style: shadow, double border, rounded corners */
void menuStyle(Menu *m, int shadow, int doubleBorder, int rounded);

/* Set position and size. Use 0 for auto-center / auto-size. */
void menuPos(Menu *m, int x, int y, int w, int h);

/* Set initially selected tab and item */
void menuInitial(Menu *m, int tab, int item);

/* Set whether to use alternate screen buffer (default: 1) */
void menuUseAltBuffer(Menu *m, int use);

/* Set whether to enable mouse tracking (default: 1) */
void menuUseMouse(Menu *m, int use);

/* Set user background draw callback. Pass NULL to disable. */
void menuBgDraw(Menu *m, MenuBgDrawFn fn, void *userData);

/* Run the interactive menu. Returns selected tab/item, or {-1,-1} on cancel. */
MenuResult menuRun(Menu *m);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // DECLARATIVE_MENU_H
