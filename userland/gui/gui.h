#ifndef OCEAN_GUI_H
#define OCEAN_GUI_H

#include "../../libs/types.h"
#include "../../kernel/drivers/display.h"
#include "../../kernel/drivers/input/key_event.h"

// Window states
typedef enum {
    WINDOW_STATE_NORMAL,
    WINDOW_STATE_MINIMIZED,
    WINDOW_STATE_MAXIMIZED,
    WINDOW_STATE_CLOSED
} WindowState;

// Window structure
typedef struct Window {
    uint32_t id;
    char title[64];
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
    uint32_t original_x;
    uint32_t original_y;
    uint32_t original_width;
    uint32_t original_height;
    WindowState state;
    bool focused;
    bool visible;
    uint32_t* buffer;
    struct Window* next;
    struct Window* prev;
} Window;

// GUI element types
typedef enum {
    GUI_ELEMENT_BUTTON,
    GUI_ELEMENT_TEXTBOX,
    GUI_ELEMENT_LABEL,
    GUI_ELEMENT_CHECKBOX,
    GUI_ELEMENT_RADIOBUTTON,
    GUI_ELEMENT_SCROLLBAR,
    GUI_ELEMENT_MENU,
    GUI_ELEMENT_TOOLBAR
} GuiElementType;

// GUI element structure
typedef struct GuiElement {
    uint32_t id;
    GuiElementType type;
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
    char text[128];
    bool enabled;
    bool visible;
    bool focused;
    void (*on_click)(struct GuiElement* element);
    void (*on_focus)(struct GuiElement* element);
    void (*on_blur)(struct GuiElement* element);
    struct GuiElement* next;
    struct GuiElement* prev;
} GuiElement;

// Desktop structure
typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t* wallpaper;
    Window* windows;
    Window* focused_window;
    GuiElement* taskbar;
    GuiElement* start_menu;
    uint32_t next_window_id;
} Desktop;

// GUI functions
void gui_init(void);
void gui_update(void);
void gui_draw_desktop(void);
void gui_handle_input(KeyEvent* event);

// Window management
Window* window_create(const char* title, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
void window_destroy(Window* window);
void window_show(Window* window);
void window_hide(Window* window);
void window_focus(Window* window);
void window_minimize(Window* window);
void window_maximize(Window* window);
void window_restore(Window* window);
void window_move(Window* window, uint32_t x, uint32_t y);
void window_resize(Window* window, uint32_t width, uint32_t height);
void window_draw(Window* window);
void window_redraw(Window* window);

// GUI element management
GuiElement* gui_element_create(GuiElementType type, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const char* text);
void gui_element_destroy(GuiElement* element);
void gui_element_show(GuiElement* element);
void gui_element_hide(GuiElement* element);
void gui_element_enable(GuiElement* element);
void gui_element_disable(GuiElement* element);
void gui_element_focus(GuiElement* element);
void gui_element_draw(GuiElement* element);

// Drawing functions
void gui_draw_button(GuiElement* button);
void gui_draw_textbox(GuiElement* textbox);
void gui_draw_label(GuiElement* label);
void gui_draw_checkbox(GuiElement* checkbox);
void gui_draw_radiobutton(GuiElement* radiobutton);
void gui_draw_scrollbar(GuiElement* scrollbar);
void gui_draw_menu(GuiElement* menu);
void gui_draw_toolbar(GuiElement* toolbar);

// Event handling
void gui_handle_mouse_click(uint32_t x, uint32_t y);
void gui_handle_mouse_move(uint32_t x, uint32_t y);
void gui_handle_key_press(KeyEvent* event);

// Desktop functions
void desktop_init(void);
void desktop_draw(void);
void desktop_set_wallpaper(const char* filename);
void desktop_show_taskbar(void);
void desktop_show_start_menu(void);

// Color definitions
#define GUI_COLOR_BACKGROUND    0x00C0C0C0
#define GUI_COLOR_BORDER        0x00808080
#define GUI_COLOR_TITLEBAR      0x000080FF
#define GUI_COLOR_BUTTON        0x00E0E0E0
#define GUI_COLOR_BUTTON_HOVER  0x00F0F0F0
#define GUI_COLOR_BUTTON_PRESS  0x00D0D0D0
#define GUI_COLOR_TEXT          0x00000000
#define GUI_COLOR_TEXT_DISABLED 0x00808080
#define GUI_COLOR_SELECTION     0x000080FF
#define GUI_COLOR_MENU          0x00F0F0F0
#define GUI_COLOR_MENU_HOVER    0x00E0E0E0

// Window decorations
#define WINDOW_TITLEBAR_HEIGHT  20
#define WINDOW_BORDER_WIDTH     1
#define WINDOW_MIN_WIDTH        100
#define WINDOW_MIN_HEIGHT       50

// Taskbar
#define TASKBAR_HEIGHT          30
#define TASKBAR_BUTTON_WIDTH    80
#define TASKBAR_BUTTON_HEIGHT   25

// Start menu
#define START_MENU_WIDTH        200
#define START_MENU_HEIGHT       300

#endif // OCEAN_GUI_H