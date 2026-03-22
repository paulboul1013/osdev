#include <stddef.h>
#include <stdint.h>

static uint16_t *const VGA=(uint16_t*)0xB8000;
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

static inline uint8_t vga_entry_color(uint8_t fg,uint8_t bg){
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char c,uint8_t color){
    return (uint16_t)c | (uint16_t)color<<8;
}

void kernel_main(void){
    const char *msg="Hello, kernel world!";
    uint8_t color=vga_entry_color(15,0);

    for(size_t i=0;msg[i]!='\0';i++){
        VGA[i]=vga_entry(msg[i],color);
    }
}