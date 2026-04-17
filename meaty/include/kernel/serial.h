#ifndef KERNEL_SERIAL_H
#define KERNEL_SERIAL_H

#include <stddef.h>

void serial_initialize(void);
int serial_is_transmit_fifo_empty(void);
void serial_putchar(char c);
int serial_write(const char *buf,size_t len);
void serial_writestring(const char *s);

#endif