#include <stddef.h>
#include <stdint.h>
#include "../include/kernel/tty.h"
#include "../include/kernel/serial.h"
#include "../arch/i386/gdt.h"


void kernel_main(void){
    terminal_initialize();
    serial_initialize();

    terminal_writestring("tty ok\n");
    serial_writestring("serial ok\n");


    terminal_writestring("loading gdt...\n");
    serial_writestring("loading gdt...\n");

    gdt_init();

    terminal_writestring("gdt loaded\n");
    serial_writestring("gdt loaded\n");
}