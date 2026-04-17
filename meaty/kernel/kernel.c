#include <stddef.h>
#include <stdint.h>
#include "../include/kernel/tty.h"
#include "../include/kernel/serial.h"


void kernel_main(void){
    terminal_initialize();
    serial_initialize();

    terminal_writestring("tty ok\n");
    serial_writestring("serial ok\n");
    serial_writestring("line 2 from COM1\n");

}