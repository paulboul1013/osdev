#ifndef KERNEL_TTY_H
#define KERNEL_TTY_H
#include <stddef.h>
void terminal_initialize(void);
void terminal_putchar(char c);
static void terminal_clear_row(size_t row);
static void terminal_scroll(void);
void terminal_writestring(const char *data);
static void terminal_update_cursor(void);
static void terminal_enable_cursor(uint8_t start,uint8_t end);
static void terminal_newline(void);
int terminal_write(const char *buf,size_t len);
#endif