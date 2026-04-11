#include <stddef.h>
#include <stdint.h>
#include "../include/kernel/tty.h"

static const size_t VGA_WIDTH=80;
static const size_t VGA_HEIGHT=25;

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
    for(size_t y=0;y<VGA_HEIGHT;y++){
        terminal_clear_row(y);
    }
}

void terminal_putchar(char c){
    if (c=='\n'){
        col=0;
        row++;

        if (row >= VGA_HEIGHT){
            terminal_scroll();
            row=VGA_HEIGHT-1;
        }
        return;
    }

    //normal character: write current cursor position
    VGA[row*VGA_WIDTH+col]=vga_entry((unsigned char)c,color);

    col++;

    //if more than VGA_WIDTH(80) , auto move to the next line
    if (col >= VGA_WIDTH){
        col=0;
        row++;
        
        if (row>=VGA_HEIGHT){
            terminal_scroll();
            row=VGA_HEIGHT-1;
        }
    }
}

static void terminal_clear_row(size_t row){
    for(size_t x=0;x<VGA_WIDTH;x++){
        VGA[row*VGA_WIDTH+x]=vga_entry(' ',color);
    }    
}

static void terminal_scroll(void){
    for(size_t y=1;y<VGA_HEIGHT;y++){
        for(size_t x=0;x<VGA_WIDTH;x++){
            VGA[(y-1)*VGA_WIDTH+x]=VGA[y*VGA_WIDTH+x];
        }
    }

    //last line empty
    terminal_clear_row(VGA_HEIGHT-1);
}

void terminal_writestring(const char *s){
    for(size_t i=0;s[i]!='\0';i++){
        terminal_putchar(s[i]);
    }
}