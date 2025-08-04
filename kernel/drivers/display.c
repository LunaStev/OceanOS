#include "display.h"
#include "../../libs/stdio.h"
#include "../kernel.h"

// Global display state
static FramebufferInfo* framebuffer = NULL;
static uint32_t* fb_ptr = NULL;
static uint32_t fb_width = 0;
static uint32_t fb_height = 0;
static uint32_t fb_pitch = 0;

// Font data (8x16 bitmap font)
static const uint8_t font_data[256][16] = {
    // Space character
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // ... (simplified font data for brevity)
};

void display_init(FramebufferInfo* fb_info) {
    framebuffer = fb_info;
    fb_ptr = (uint32_t*)fb_info->base_address;
    fb_width = fb_info->width;
    fb_height = fb_info->height;
    fb_pitch = fb_info->pixels_per_scan_line;
    
    console_printf("Display initialized: %dx%d\n", fb_width, fb_height);
}

void display_clear(uint32_t color) {
    for (uint32_t y = 0; y < fb_height; y++) {
        for (uint32_t x = 0; x < fb_width; x++) {
            fb_ptr[y * fb_pitch + x] = color;
        }
    }
}

void display_put_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (x < fb_width && y < fb_height) {
        fb_ptr[y * fb_pitch + x] = color;
    }
}

void display_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    for (uint32_t py = y; py < y + height && py < fb_height; py++) {
        for (uint32_t px = x; px < x + width && px < fb_width; px++) {
            fb_ptr[py * fb_pitch + px] = color;
        }
    }
}

void display_draw_char(uint32_t x, uint32_t y, char c, uint32_t color) {
    if (x + 8 > fb_width || y + 16 > fb_height) return;
    
    const uint8_t* char_data = font_data[(uint8_t)c];
    
    for (uint32_t py = 0; py < 16; py++) {
        uint8_t row = char_data[py];
        for (uint32_t px = 0; px < 8; px++) {
            if (row & (0x80 >> px)) {
                display_put_pixel(x + px, y + py, color);
            }
        }
    }
}

void display_draw_string(uint32_t x, uint32_t y, const char* str, uint32_t color) {
    uint32_t current_x = x;
    while (*str) {
        if (*str == '\n') {
            y += 16;
            current_x = x;
        } else {
            display_draw_char(current_x, y, *str, color);
            current_x += 8;
        }
        str++;
    }
}

void display_draw_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t color) {
    int dx = abs((int)x2 - (int)x1);
    int dy = abs((int)y2 - (int)y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx - dy;
    
    uint32_t x = x1;
    uint32_t y = y1;
    
    while (1) {
        display_put_pixel(x, y, color);
        
        if (x == x2 && y == y2) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
}

void display_draw_circle(uint32_t center_x, uint32_t center_y, uint32_t radius, uint32_t color) {
    int x = radius;
    int y = 0;
    int err = 0;
    
    while (x >= y) {
        display_put_pixel(center_x + x, center_y + y, color);
        display_put_pixel(center_x + y, center_y + x, color);
        display_put_pixel(center_x - y, center_y + x, color);
        display_put_pixel(center_x - x, center_y + y, color);
        display_put_pixel(center_x - x, center_y - y, color);
        display_put_pixel(center_x - y, center_y - x, color);
        display_put_pixel(center_x + y, center_y - x, color);
        display_put_pixel(center_x + x, center_y - y, color);
        
        if (err <= 0) {
            y += 1;
            err += 2*y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2*x + 1;
        }
    }
}

void display_fill_circle(uint32_t center_x, uint32_t center_y, uint32_t radius, uint32_t color) {
    for (uint32_t y = 0; y < fb_height; y++) {
        for (uint32_t x = 0; x < fb_width; x++) {
            uint32_t dx = x - center_x;
            uint32_t dy = y - center_y;
            if (dx*dx + dy*dy <= radius*radius) {
                display_put_pixel(x, y, color);
            }
        }
    }
}

void display_draw_triangle(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, 
                          uint32_t x3, uint32_t y3, uint32_t color) {
    display_draw_line(x1, y1, x2, y2, color);
    display_draw_line(x2, y2, x3, y3, color);
    display_draw_line(x3, y3, x1, y1, color);
}

void display_fill_triangle(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, 
                          uint32_t x3, uint32_t y3, uint32_t color) {
    // Simple triangle filling algorithm
    uint32_t min_y = MIN(MIN(y1, y2), y3);
    uint32_t max_y = MAX(MAX(y1, y2), y3);
    
    for (uint32_t y = min_y; y <= max_y; y++) {
        uint32_t x_start = fb_width;
        uint32_t x_end = 0;
        
        // Find intersection points with each edge
        if (y1 != y2) {
            uint32_t x = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
            if (x < x_start) x_start = x;
            if (x > x_end) x_end = x;
        }
        if (y2 != y3) {
            uint32_t x = x2 + (x3 - x2) * (y - y2) / (y3 - y2);
            if (x < x_start) x_start = x;
            if (x > x_end) x_end = x;
        }
        if (y3 != y1) {
            uint32_t x = x3 + (x1 - x3) * (y - y3) / (y1 - y3);
            if (x < x_start) x_start = x;
            if (x > x_end) x_end = x;
        }
        
        // Draw horizontal line
        for (uint32_t x = x_start; x <= x_end; x++) {
            display_put_pixel(x, y, color);
        }
    }
}

void display_copy_region(uint32_t src_x, uint32_t src_y, uint32_t width, uint32_t height,
                        uint32_t dest_x, uint32_t dest_y) {
    for (uint32_t y = 0; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
            uint32_t src_pixel = fb_ptr[(src_y + y) * fb_pitch + (src_x + x)];
            display_put_pixel(dest_x + x, dest_y + y, src_pixel);
        }
    }
}

void display_scroll(uint32_t lines, uint32_t color) {
    // Scroll the display up by copying pixels
    for (uint32_t y = 0; y < fb_height - lines * 16; y++) {
        for (uint32_t x = 0; x < fb_width; x++) {
            fb_ptr[y * fb_pitch + x] = fb_ptr[(y + lines * 16) * fb_pitch + x];
        }
    }
    
    // Clear the bottom lines
    for (uint32_t y = fb_height - lines * 16; y < fb_height; y++) {
        for (uint32_t x = 0; x < fb_width; x++) {
            fb_ptr[y * fb_pitch + x] = color;
        }
    }
}

uint32_t display_get_width(void) {
    return fb_width;
}

uint32_t display_get_height(void) {
    return fb_height;
}

uint32_t display_get_pitch(void) {
    return fb_pitch;
}

FramebufferInfo* display_get_framebuffer(void) {
    return framebuffer;
}