#include <stddef.h>
#include <stdint.h>
#include "../include/kernel/tty.h"


static size_t row;
static size_t col;
static uint8_t color;
static uint16_t* const VGA=(uint16_t*)0xB8000;

static inline uint16_t vga_entry(char c,uint8_t color){
    return (uint16_t)c | (uint16_t)color<<8;
}

void terminal_initialize(void){
    row=0;
    col=0;
    color=0x0F;
    for(size_t y=0;y<25;y++){
        for(size_t x=0;x<80;x++){
            VGA[y*80+x]=vga_entry(' ',color);
        }
    }
}

void terminal_writestring(const char *s){
    for(size_t i=0;s[i];i++){
        VGA[row*80+col]=vga_entry(s[i],color);
        col++;
    }
}