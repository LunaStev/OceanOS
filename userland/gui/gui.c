#include "gui.h"
#include "../../libs/stdio.h"
#include "../../kernel/memory/memory.h"

// Global GUI state
static Desktop desktop;
static uint32_t mouse_x = 0;
static uint32_t mouse_y = 0;
static bool mouse_left_pressed = false;
static bool mouse_right_pressed = false;

void gui_init(void) {
    // Initialize desktop
    desktop.width = display_get_width();
    desktop.height = display_get_height();
    desktop.windows = NULL;
    desktop.focused_window = NULL;
    desktop.taskbar = NULL;
    desktop.start_menu = NULL;
    desktop.next_window_id = 1;
    
    // Create default wallpaper (blue gradient)
    desktop.wallpaper = (uint32_t*)memory_allocate(desktop.width * desktop.height * 4);
    for (uint32_t y = 0; y < desktop.height; y++) {
        for (uint32_t x = 0; x < desktop.width; x++) {
            uint8_t blue = (y * 255) / desktop.height;
            desktop.wallpaper[y * desktop.width + x] = 0xFF000000 | (blue << 16);
        }
    }
    
    // Create taskbar
    desktop.taskbar = gui_element_create(GUI_ELEMENT_TOOLBAR, 0, desktop.height - TASKBAR_HEIGHT, 
                                       desktop.width, TASKBAR_HEIGHT, "Taskbar");
    
    // Create start menu button
    GuiElement* start_button = gui_element_create(GUI_ELEMENT_BUTTON, 5, desktop.height - TASKBAR_HEIGHT + 2,
                                                 TASKBAR_BUTTON_WIDTH, TASKBAR_BUTTON_HEIGHT, "Start");
    start_button->on_click = start_menu_click;
    
    console_puts("GUI system initialized\n");
}

void gui_update(void) {
    // Process input events
    while (key_event_available()) {
        KeyEvent event = key_event_dequeue();
        gui_handle_key_press(&event);
    }
    
    // Update windows
    Window* window = desktop.windows;
    while (window) {
        if (window->visible) {
            window_draw(window);
        }
        window = window->next;
    }
    
    // Draw desktop
    desktop_draw();
}

void gui_draw_desktop(void) {
    // Draw wallpaper
    uint32_t* fb = (uint32_t*)display_get_framebuffer()->base_address;
    uint32_t pitch = display_get_pitch();
    
    for (uint32_t y = 0; y < desktop.height; y++) {
        for (uint32_t x = 0; x < desktop.width; x++) {
            fb[y * pitch + x] = desktop.wallpaper[y * desktop.width + x];
        }
    }
    
    // Draw taskbar
    if (desktop.taskbar) {
        gui_element_draw(desktop.taskbar);
    }
}

Window* window_create(const char* title, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    Window* window = (Window*)memory_allocate(sizeof(Window));
    if (!window) return NULL;
    
    window->id = desktop.next_window_id++;
    strncpy(window->title, title, 63);
    window->title[63] = '\0';
    window->x = x;
    window->y = y;
    window->width = width;
    window->height = height;
    window->original_x = x;
    window->original_y = y;
    window->original_width = width;
    window->original_height = height;
    window->state = WINDOW_STATE_NORMAL;
    window->focused = false;
    window->visible = true;
    
    // Allocate window buffer
    window->buffer = (uint32_t*)memory_allocate(width * height * 4);
    if (!window->buffer) {
        memory_free(window);
        return NULL;
    }
    
    // Add to window list
    window->next = desktop.windows;
    window->prev = NULL;
    if (desktop.windows) {
        desktop.windows->prev = window;
    }
    desktop.windows = window;
    
    return window;
}

void window_destroy(Window* window) {
    if (!window) return;
    
    // Remove from window list
    if (window->prev) {
        window->prev->next = window->next;
    } else {
        desktop.windows = window->next;
    }
    if (window->next) {
        window->next->prev = window->prev;
    }
    
    // Free window buffer
    if (window->buffer) {
        memory_free(window->buffer);
    }
    
    // Free window structure
    memory_free(window);
}

void window_show(Window* window) {
    if (window) {
        window->visible = true;
    }
}

void window_hide(Window* window) {
    if (window) {
        window->visible = false;
    }
}

void window_focus(Window* window) {
    if (desktop.focused_window) {
        desktop.focused_window->focused = false;
    }
    
    desktop.focused_window = window;
    if (window) {
        window->focused = true;
        
        // Move to front
        if (window->prev || window->next) {
            // Remove from current position
            if (window->prev) {
                window->prev->next = window->next;
            } else {
                desktop.windows = window->next;
            }
            if (window->next) {
                window->next->prev = window->prev;
            }
            
            // Add to front
            window->next = desktop.windows;
            window->prev = NULL;
            if (desktop.windows) {
                desktop.windows->prev = window;
            }
            desktop.windows = window;
        }
    }
}

void window_minimize(Window* window) {
    if (window) {
        window->state = WINDOW_STATE_MINIMIZED;
        window->visible = false;
    }
}

void window_maximize(Window* window) {
    if (window) {
        window->original_x = window->x;
        window->original_y = window->y;
        window->original_width = window->width;
        window->original_height = window->height;
        
        window->x = 0;
        window->y = WINDOW_TITLEBAR_HEIGHT;
        window->width = desktop.width;
        window->height = desktop.height - TASKBAR_HEIGHT - WINDOW_TITLEBAR_HEIGHT;
        window->state = WINDOW_STATE_MAXIMIZED;
    }
}

void window_restore(Window* window) {
    if (window) {
        window->x = window->original_x;
        window->y = window->original_y;
        window->width = window->original_width;
        window->height = window->original_height;
        window->state = WINDOW_STATE_NORMAL;
    }
}

void window_move(Window* window, uint32_t x, uint32_t y) {
    if (window) {
        window->x = x;
        window->y = y;
    }
}

void window_resize(Window* window, uint32_t width, uint32_t height) {
    if (window && width >= WINDOW_MIN_WIDTH && height >= WINDOW_MIN_HEIGHT) {
        window->width = width;
        window->height = height;
        
        // Reallocate buffer if needed
        if (window->buffer) {
            memory_free(window->buffer);
        }
        window->buffer = (uint32_t*)memory_allocate(width * height * 4);
    }
}

void window_redraw(Window* window) {
    window_draw(window);
}

void window_draw(Window* window) {
    if (!window || !window->visible) return;
    
    uint32_t* fb = (uint32_t*)display_get_framebuffer()->base_address;
    uint32_t pitch = display_get_pitch();
    
    // Draw window border
    uint32_t border_color = window->focused ? GUI_COLOR_TITLEBAR : GUI_COLOR_BORDER;
    
    // Top border
    for (uint32_t x = window->x; x < window->x + window->width; x++) {
        display_put_pixel(x, window->y, border_color);
    }
    
    // Bottom border
    for (uint32_t x = window->x; x < window->x + window->width; x++) {
        display_put_pixel(x, window->y + window->height - 1, border_color);
    }
    
    // Left border
    for (uint32_t y = window->y; y < window->y + window->height; y++) {
        display_put_pixel(window->x, y, border_color);
    }
    
    // Right border
    for (uint32_t x = window->x + window->width - 1; x < window->x + window->width; x++) {
        for (uint32_t y = window->y; y < window->y + window->height; y++) {
            display_put_pixel(x, y, border_color);
        }
    }
    
    // Draw title bar
    uint32_t titlebar_color = window->focused ? GUI_COLOR_TITLEBAR : GUI_COLOR_BORDER;
    for (uint32_t y = window->y + 1; y < window->y + WINDOW_TITLEBAR_HEIGHT; y++) {
        for (uint32_t x = window->x + 1; x < window->x + window->width - 1; x++) {
            display_put_pixel(x, y, titlebar_color);
        }
    }
    
    // Draw title text
    display_draw_string(window->x + 5, window->y + 3, window->title, GUI_COLOR_TEXT);
    
    // Draw window content
    for (uint32_t y = 0; y < window->height - WINDOW_TITLEBAR_HEIGHT - 2; y++) {
        for (uint32_t x = 0; x < window->width - 2; x++) {
            uint32_t pixel = window->buffer[y * (window->width - 2) + x];
            display_put_pixel(window->x + x + 1, window->y + y + WINDOW_TITLEBAR_HEIGHT, pixel);
        }
    }
}

GuiElement* gui_element_create(GuiElementType type, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const char* text) {
    GuiElement* element = (GuiElement*)memory_allocate(sizeof(GuiElement));
    if (!element) return NULL;
    
    element->id = 0; // Will be assigned by parent
    element->type = type;
    element->x = x;
    element->y = y;
    element->width = width;
    element->height = height;
    strncpy(element->text, text, 127);
    element->text[127] = '\0';
    element->enabled = true;
    element->visible = true;
    element->focused = false;
    element->on_click = NULL;
    element->on_focus = NULL;
    element->on_blur = NULL;
    element->next = NULL;
    element->prev = NULL;
    
    return element;
}

void gui_element_destroy(GuiElement* element) {
    if (element) {
        memory_free(element);
    }
}

void gui_element_show(GuiElement* element) {
    if (element) {
        element->visible = true;
    }
}

void gui_element_hide(GuiElement* element) {
    if (element) {
        element->visible = false;
    }
}

void gui_element_enable(GuiElement* element) {
    if (element) {
        element->enabled = true;
    }
}

void gui_element_disable(GuiElement* element) {
    if (element) {
        element->enabled = false;
    }
}

void gui_element_focus(GuiElement* element) {
    if (element) {
        element->focused = true;
        if (element->on_focus) {
            element->on_focus(element);
        }
    }
}

void gui_element_draw(GuiElement* element) {
    if (!element || !element->visible) return;
    
    switch (element->type) {
        case GUI_ELEMENT_BUTTON:
            gui_draw_button(element);
            break;
        case GUI_ELEMENT_TEXTBOX:
            gui_draw_textbox(element);
            break;
        case GUI_ELEMENT_LABEL:
            gui_draw_label(element);
            break;
        case GUI_ELEMENT_TOOLBAR:
            gui_draw_toolbar(element);
            break;
        default:
            break;
    }
}

void gui_draw_button(GuiElement* button) {
    uint32_t color = GUI_COLOR_BUTTON;
    if (!button->enabled) {
        color = GUI_COLOR_TEXT_DISABLED;
    }
    
    // Draw button background
    for (uint32_t y = button->y; y < button->y + button->height; y++) {
        for (uint32_t x = button->x; x < button->x + button->width; x++) {
            display_put_pixel(x, y, color);
        }
    }
    
    // Draw button border
    for (uint32_t x = button->x; x < button->x + button->width; x++) {
        display_put_pixel(x, button->y, GUI_COLOR_BORDER);
        display_put_pixel(x, button->y + button->height - 1, GUI_COLOR_BORDER);
    }
    for (uint32_t y = button->y; y < button->y + button->height; y++) {
        display_put_pixel(button->x, y, GUI_COLOR_BORDER);
        display_put_pixel(button->x + button->width - 1, y, GUI_COLOR_BORDER);
    }
    
    // Draw button text
    uint32_t text_color = button->enabled ? GUI_COLOR_TEXT : GUI_COLOR_TEXT_DISABLED;
    uint32_t text_x = button->x + (button->width - strlen(button->text) * 8) / 2;
    uint32_t text_y = button->y + (button->height - 16) / 2;
    display_draw_string(text_x, text_y, button->text, text_color);
}

void gui_draw_textbox(GuiElement* textbox) {
    // Draw textbox background
    for (uint32_t y = textbox->y; y < textbox->y + textbox->height; y++) {
        for (uint32_t x = textbox->x; x < textbox->x + textbox->width; x++) {
            display_put_pixel(x, y, GUI_COLOR_WHITE);
        }
    }
    
    // Draw textbox border
    for (uint32_t x = textbox->x; x < textbox->x + textbox->width; x++) {
        display_put_pixel(x, textbox->y, GUI_COLOR_BORDER);
        display_put_pixel(x, textbox->y + textbox->height - 1, GUI_COLOR_BORDER);
    }
    for (uint32_t y = textbox->y; y < textbox->y + textbox->height; y++) {
        display_put_pixel(textbox->x, y, GUI_COLOR_BORDER);
        display_put_pixel(textbox->x + textbox->width - 1, y, GUI_COLOR_BORDER);
    }
    
    // Draw text
    display_draw_string(textbox->x + 2, textbox->y + 2, textbox->text, GUI_COLOR_TEXT);
}

void gui_draw_label(GuiElement* label) {
    display_draw_string(label->x, label->y, label->text, GUI_COLOR_TEXT);
}

void gui_draw_checkbox(GuiElement* checkbox) {
    // Draw checkbox box
    for (uint32_t y = checkbox->y; y < checkbox->y + 16; y++) {
        for (uint32_t x = checkbox->x; x < checkbox->x + 16; x++) {
            display_put_pixel(x, y, GUI_COLOR_WHITE);
        }
    }
    
    // Draw checkbox border
    for (uint32_t x = checkbox->x; x < checkbox->x + 16; x++) {
        display_put_pixel(x, checkbox->y, GUI_COLOR_BORDER);
        display_put_pixel(x, checkbox->y + 15, GUI_COLOR_BORDER);
    }
    for (uint32_t y = checkbox->y; y < checkbox->y + 16; y++) {
        display_put_pixel(checkbox->x, y, GUI_COLOR_BORDER);
        display_put_pixel(checkbox->x + 15, y, GUI_COLOR_BORDER);
    }
    
    // Draw text
    display_draw_string(checkbox->x + 20, checkbox->y, checkbox->text, GUI_COLOR_TEXT);
}

void gui_draw_radiobutton(GuiElement* radiobutton) {
    // Draw radio button circle
    display_draw_circle(radiobutton->x + 8, radiobutton->y + 8, 8, GUI_COLOR_BORDER);
    
    // Draw text
    display_draw_string(radiobutton->x + 20, radiobutton->y, radiobutton->text, GUI_COLOR_TEXT);
}

void gui_draw_scrollbar(GuiElement* scrollbar) {
    // Draw scrollbar background
    for (uint32_t y = scrollbar->y; y < scrollbar->y + scrollbar->height; y++) {
        for (uint32_t x = scrollbar->x; x < scrollbar->x + scrollbar->width; x++) {
            display_put_pixel(x, y, GUI_COLOR_MENU);
        }
    }
    
    // Draw scrollbar border
    for (uint32_t x = scrollbar->x; x < scrollbar->x + scrollbar->width; x++) {
        display_put_pixel(x, scrollbar->y, GUI_COLOR_BORDER);
        display_put_pixel(x, scrollbar->y + scrollbar->height - 1, GUI_COLOR_BORDER);
    }
}

void gui_draw_menu(GuiElement* menu) {
    // Draw menu background
    for (uint32_t y = menu->y; y < menu->y + menu->height; y++) {
        for (uint32_t x = menu->x; x < menu->x + menu->width; x++) {
            display_put_pixel(x, y, GUI_COLOR_MENU);
        }
    }
    
    // Draw menu border
    for (uint32_t x = menu->x; x < menu->x + menu->width; x++) {
        display_put_pixel(x, menu->y, GUI_COLOR_BORDER);
        display_put_pixel(x, menu->y + menu->height - 1, GUI_COLOR_BORDER);
    }
    for (uint32_t y = menu->y; y < menu->y + menu->height; y++) {
        display_put_pixel(menu->x, y, GUI_COLOR_BORDER);
        display_put_pixel(menu->x + menu->width - 1, y, GUI_COLOR_BORDER);
    }
    
    // Draw menu title
    display_draw_string(menu->x + 5, menu->y + 5, menu->text, GUI_COLOR_TEXT);
}

void gui_draw_toolbar(GuiElement* toolbar) {
    // Draw toolbar background
    for (uint32_t y = toolbar->y; y < toolbar->y + toolbar->height; y++) {
        for (uint32_t x = toolbar->x; x < toolbar->x + toolbar->width; x++) {
            display_put_pixel(x, y, GUI_COLOR_MENU);
        }
    }
    
    // Draw top border
    for (uint32_t x = toolbar->x; x < toolbar->x + toolbar->width; x++) {
        display_put_pixel(x, toolbar->y, GUI_COLOR_BORDER);
    }
}

void gui_handle_mouse_click(uint32_t x, uint32_t y) {
    // Handle mouse click events
    // This would be implemented to handle window and GUI element clicks
}

void gui_handle_mouse_move(uint32_t x, uint32_t y) {
    mouse_x = x;
    mouse_y = y;
}

void gui_handle_key_press(KeyEvent* event) {
    if (!event || !event->pressed) return;
    
    // Handle global shortcuts
    if (event->scancode == KEY_ESCAPE) {
        // Close focused window or start menu
        if (desktop.start_menu && desktop.start_menu->visible) {
            desktop.start_menu->visible = false;
        } else if (desktop.focused_window) {
            window_hide(desktop.focused_window);
        }
    }
    
    // Handle window-specific input
    if (desktop.focused_window) {
        // Pass to focused window
        // This would be implemented by the specific window
    }
}

void start_menu_click(GuiElement* element) {
    if (!desktop.start_menu) {
        desktop.start_menu = gui_element_create(GUI_ELEMENT_MENU, 5, 
                                               desktop.height - TASKBAR_HEIGHT - START_MENU_HEIGHT,
                                               START_MENU_WIDTH, START_MENU_HEIGHT, "Start Menu");
    }
    
    desktop.start_menu->visible = !desktop.start_menu->visible;
}

void desktop_init(void) {
    gui_init();
}

void desktop_draw(void) {
    gui_draw_desktop();
}

void desktop_set_wallpaper(const char* filename) {
    // This would load a wallpaper image file
    // For now, just use the default gradient
}

void desktop_show_taskbar(void) {
    if (desktop.taskbar) {
        gui_element_show(desktop.taskbar);
    }
}

void desktop_show_start_menu(void) {
    if (desktop.start_menu) {
        desktop.start_menu->visible = true;
    }
} 