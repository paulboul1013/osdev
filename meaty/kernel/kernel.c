#include <stddef.h>
#include <stdint.h>
#include "../include/kernel/tty.h"


void kernel_main(void){
    terminal_initialize();
    terminal_writestring("line 1\n");
    terminal_writestring("line 2\n");
    terminal_writestring("line 3\n");
    terminal_writestring("abc\ndef\nxyz\n");
    terminal_writestring("This is a very long line that should continue writing until it wraps to the next VGA row automatically.");

}