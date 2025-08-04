#ifndef OCEAN_STDIO_H
#define OCEAN_STDIO_H

#include "types.h"

// Console colors
#define COLOR_BLACK         0x00
#define COLOR_BLUE          0x01
#define COLOR_GREEN         0x02
#define COLOR_CYAN          0x03
#define COLOR_RED           0x04
#define COLOR_MAGENTA       0x05
#define COLOR_BROWN         0x06
#define COLOR_LIGHT_GREY    0x07
#define COLOR_DARK_GREY     0x08
#define COLOR_LIGHT_BLUE    0x09
#define COLOR_LIGHT_GREEN   0x0A
#define COLOR_LIGHT_CYAN    0x0B
#define COLOR_LIGHT_RED     0x0C
#define COLOR_LIGHT_MAGENTA 0x0D
#define COLOR_LIGHT_BROWN   0x0E
#define COLOR_WHITE         0x0F

// Console functions
void console_init(void);
void console_clear(void);
void console_set_color(uint8_t foreground, uint8_t background);
void console_putchar(char c);
void console_puts(const char* str);
void console_printf(const char* format, ...);

// String functions
size_t strlen(const char* str);
void strcpy(char* dest, const char* src);
void strncpy(char* dest, const char* src, size_t n);
int strcmp(const char* str1, const char* str2);
int strncmp(const char* str1, const char* str2, size_t n);
char* strcat(char* dest, const char* src);
char* strchr(const char* str, int c);
char* strrchr(const char* str, int c);

// Memory functions
void* memset(void* ptr, int value, size_t num);
void* memcpy(void* dest, const void* src, size_t num);
void* memmove(void* dest, const void* src, size_t num);
int memcmp(const void* ptr1, const void* ptr2, size_t num);

// Type definitions
typedef unsigned long size_t;

#endif // OCEAN_STDIO_H