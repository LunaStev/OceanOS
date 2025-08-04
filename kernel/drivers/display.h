#ifndef OCEAN_DISPLAY_H
#define OCEAN_DISPLAY_H

#include "../../libs/types.h"
#include "../../bootloader/framebuffer.h"

// Display functions
void display_init(FramebufferInfo* fb_info);
void display_clear(uint32_t color);
void display_put_pixel(uint32_t x, uint32_t y, uint32_t color);
void display_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
void display_draw_char(uint32_t x, uint32_t y, char c, uint32_t color);
void display_draw_string(uint32_t x, uint32_t y, const char* str, uint32_t color);

// Advanced drawing functions
void display_draw_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t color);
void display_draw_circle(uint32_t center_x, uint32_t center_y, uint32_t radius, uint32_t color);
void display_fill_circle(uint32_t center_x, uint32_t center_y, uint32_t radius, uint32_t color);
void display_draw_triangle(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, 
                          uint32_t x3, uint32_t y3, uint32_t color);
void display_fill_triangle(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, 
                          uint32_t x3, uint32_t y3, uint32_t color);

// Utility functions
void display_copy_region(uint32_t src_x, uint32_t src_y, uint32_t width, uint32_t height,
                        uint32_t dest_x, uint32_t dest_y);
void display_scroll(uint32_t lines, uint32_t color);

// Information functions
uint32_t display_get_width(void);
uint32_t display_get_height(void);
uint32_t display_get_pitch(void);
FramebufferInfo* display_get_framebuffer(void);

#endif // OCEAN_DISPLAY_H 