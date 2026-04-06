#ifndef KERNEL_TTY_H
#define KERNEL_TTY_H
#include <stddef.h>
void terminal_initialize(void);
void terminal_writestring(const char *data);
#endif