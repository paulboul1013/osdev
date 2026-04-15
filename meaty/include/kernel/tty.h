#ifndef KERNEL_TTY_H
#define KERNEL_TTY_H
#include <stddef.h>
void terminal_initialize(void);
void terminal_putchar(char c);
void terminal_writestring(const char *data);
size_t terminal_write(const char *buf,size_t len);
#endif