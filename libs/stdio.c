#include "stdio.h"

// Console state
static uint16_t* video_memory = (uint16_t*)0xB8000;
static int console_x = 0;
static int console_y = 0;
static uint8_t console_fg_color = COLOR_WHITE;
static uint8_t console_bg_color = COLOR_BLACK;

void console_init(void) {
    console_x = 0;
    console_y = 0;
    console_fg_color = COLOR_WHITE;
    console_bg_color = COLOR_BLACK;
    console_clear();
}

void console_clear(void) {
    for (int i = 0; i < 80 * 25; i++) {
        video_memory[i] = (console_bg_color << 12) | (console_fg_color << 8) | ' ';
    }
    console_x = 0;
    console_y = 0;
}

void console_set_color(uint8_t foreground, uint8_t background) {
    console_fg_color = foreground;
    console_bg_color = background;
}

void console_putchar(char c) {
    if (c == '\n') {
        console_x = 0;
        console_y++;
        if (console_y >= 25) {
            // Scroll up
            for (int i = 0; i < 80 * 24; i++) {
                video_memory[i] = video_memory[i + 80];
            }
            for (int i = 80 * 24; i < 80 * 25; i++) {
                video_memory[i] = (console_bg_color << 12) | (console_fg_color << 8) | ' ';
            }
            console_y = 24;
        }
        return;
    }
    
    if (c == '\r') {
        console_x = 0;
        return;
    }
    
    if (c == '\t') {
        console_x = (console_x + 8) & ~7;
        if (console_x >= 80) {
            console_x = 0;
            console_y++;
        }
        return;
    }
    
    int index = console_y * 80 + console_x;
    video_memory[index] = (console_bg_color << 12) | (console_fg_color << 8) | c;
    
    console_x++;
    if (console_x >= 80) {
        console_x = 0;
        console_y++;
        if (console_y >= 25) {
            // Scroll up
            for (int i = 0; i < 80 * 24; i++) {
                video_memory[i] = video_memory[i + 80];
            }
            for (int i = 80 * 24; i < 80 * 25; i++) {
                video_memory[i] = (console_bg_color << 12) | (console_fg_color << 8) | ' ';
            }
            console_y = 24;
        }
    }
}

void console_puts(const char* str) {
    while (*str) {
        console_putchar(*str++);
    }
}

void console_printf(const char* format, ...) {
    // Simple implementation - just print the format string
    console_puts(format);
}

// String functions
size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

void strcpy(char* dest, const char* src) {
    while (*src) {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';
}

void strncpy(char* dest, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i]; i++) {
        dest[i] = src[i];
    }
    for (; i < n; i++) {
        dest[i] = '\0';
    }
}

int strcmp(const char* str1, const char* str2) {
    while (*str1 && *str2 && *str1 == *str2) {
        str1++;
        str2++;
    }
    return (int)(*str1 - *str2);
}

int strncmp(const char* str1, const char* str2, size_t n) {
    size_t i;
    for (i = 0; i < n && str1[i] && str2[i] && str1[i] == str2[i]; i++);
    if (i == n) return 0;
    return (int)(str1[i] - str2[i]);
}

char* strcat(char* dest, const char* src) {
    char* ptr = dest + strlen(dest);
    while (*src) {
        *ptr = *src;
        ptr++;
        src++;
    }
    *ptr = '\0';
    return dest;
}

char* strchr(const char* str, int c) {
    while (*str) {
        if (*str == c) return (char*)str;
        str++;
    }
    return NULL;
}

char* strrchr(const char* str, int c) {
    char* last = NULL;
    while (*str) {
        if (*str == c) last = (char*)str;
        str++;
    }
    return last;
}

// Memory functions
void* memset(void* ptr, int value, size_t num) {
    unsigned char* p = (unsigned char*)ptr;
    for (size_t i = 0; i < num; i++) {
        p[i] = (unsigned char)value;
    }
    return ptr;
}

void* memcpy(void* dest, const void* src, size_t num) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    for (size_t i = 0; i < num; i++) {
        d[i] = s[i];
    }
    return dest;
}

void* memmove(void* dest, const void* src, size_t num) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    
    if (d < s) {
        for (size_t i = 0; i < num; i++) {
            d[i] = s[i];
        }
    } else {
        for (size_t i = num; i > 0; i--) {
            d[i-1] = s[i-1];
        }
    }
    return dest;
}

int memcmp(const void* ptr1, const void* ptr2, size_t num) {
    const unsigned char* p1 = (const unsigned char*)ptr1;
    const unsigned char* p2 = (const unsigned char*)ptr2;
    
    for (size_t i = 0; i < num; i++) {
        if (p1[i] != p2[i]) {
            return (int)(p1[i] - p2[i]);
        }
    }
    return 0;
} 