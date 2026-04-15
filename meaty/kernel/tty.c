#include <stddef.h>
#include <stdint.h>
#include "../include/kernel/tty.h"
#include "../include/kernel/io.h"

static const size_t VGA_WIDTH=80;
static const size_t VGA_HEIGHT=25;
static const size_t TAB_WIDTH=4;

static size_t row;
static size_t col;
static uint8_t color;
static volatile uint16_t* const VGA=(uint16_t*)0xB8000;

//VGA cursor ports
#define VGA_CMD_PORT 0x3D4
#define VGA_DATA_PORT 0x3D5

#define CURSOR_HIGH_BYTE 14
#define CURSOR_LOW_BYTE 15

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

    //underline-like cursor
    terminal_enable_cursor(14,15);
    terminal_update_cursor();
}

static void terminal_newline(void){
    col=0;
    row++;

    if (row>=VGA_HEIGHT){
        terminal_scroll();
        row=VGA_HEIGHT-1;
    }
}

void terminal_putchar(char c){
    if (c=='\n'){
        terminal_newline();
        terminal_update_cursor();
        return;
    }

    if (c=='\r'){
        col=0;
        terminal_update_cursor();
        return;
    }

    if (c=='\t'){
        size_t next_tab_stop=((col/TAB_WIDTH)+1)*TAB_WIDTH;
        
        while(col<next_tab_stop){
            terminal_putchar(' ');
        }
        return;
    }

    //normal character: write current cursor position
    VGA[row*VGA_WIDTH+col]=vga_entry((unsigned char)c,color);

    col++;

    //if more than VGA_WIDTH(80) , auto move to the next line
    if (col >= VGA_WIDTH){
        terminal_newline();
    }

    terminal_update_cursor();
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
    size_t len=0;
    while(s[len]!='\0'){
        len++;
    }
    terminal_write(s,len);
}

int terminal_write(const char *buf,size_t len){
    for(size_t i=0;i<len;i++){
        terminal_putchar(buf[i]);
    }
    return len;
}

static void terminal_update_cursor(void){
    uint16_t pos=(uint16_t)(row*VGA_WIDTH+col);

    outb(VGA_CMD_PORT,CURSOR_HIGH_BYTE);
    outb(VGA_DATA_PORT,(uint8_t)((pos>>8)&0xFF));

    outb(VGA_CMD_PORT,CURSOR_LOW_BYTE);
    outb(VGA_DATA_PORT,(uint8_t)(pos&0xFF));
}

static void terminal_enable_cursor(uint8_t start,uint8_t end){
    outb(VGA_CMD_PORT,0x0A);
    outb(VGA_DATA_PORT,(inb(VGA_DATA_PORT)&0xC0)|start);

    outb(VGA_CMD_PORT,0x0B);
    outb(VGA_DATA_PORT,(inb(VGA_DATA_PORT)&0xE0)|end);
}

