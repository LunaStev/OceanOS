// kernel/include/framebuffer.h
#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "../libs/types.h"

typedef struct {
    uint64_t base_address;
    uint64_t buffer_size;
    uint32_t width;
    uint32_t height;
    uint32_t pixels_per_scan_line;
    uint32_t bits_per_pixel;
} FramebufferInfo;

// Color definitions
#define COLOR_BLACK     0x00000000
#define COLOR_WHITE     0xFFFFFFFF
#define COLOR_RED       0x00FF0000
#define COLOR_GREEN     0x0000FF00
#define COLOR_BLUE      0x000000FF
#define COLOR_CYAN      0x0000FFFF
#define COLOR_MAGENTA   0x00FF00FF
#define COLOR_YELLOW    0x00FFFF00

// Framebuffer functions
void framebuffer_init(FramebufferInfo* fb_info);
void framebuffer_clear(uint32_t color);
void framebuffer_put_pixel(uint32_t x, uint32_t y, uint32_t color);
void framebuffer_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
void framebuffer_draw_char(uint32_t x, uint32_t y, char c, uint32_t color);
void framebuffer_draw_string(uint32_t x, uint32_t y, const char* str, uint32_t color);

#endif // FRAMEBUFFER_H
