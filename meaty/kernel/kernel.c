#include <stddef.h>
#include <stdint.h>
#include "../include/kernel/tty.h"


void kernel_main(void){
    terminal_initialize();
    terminal_writestring("terminal layer ok");
}