#include <stddef.h>
#include <stdint.h>
#include "../include/kernel/serial.h"
#include "../include/kernel/io.h"

#define SERIAL_COM1_BASE 0x3F8

#define SERIAL_DATA_PORT(base) (base)
#define SERIAL_INTERRUPT_ENABLE(base) (base+1)
#define SERIAL_FIFO_COMMAND_PORT(base) (base+2)
#define SERIAL_LINE_COMMAND_PORT(base) (base+3)
#define SERIAL_MODEM_COMMAND_PORT(base) (base+4)
#define SERIAL_LINE_STATUS_PORT(base) (base+5)


//line control
#define SERIAL_LINE_ENABLE_DLAB 0x80
#define SERIAL_LINE_8N1 0x03

//FIFO control
#define SERIAL_FIFO_ENABLE_CLEAR_14 0xC7

//Modem control
#define SERIAL_MODEM_DTR_RTS 0x03

//Line status
#define SERIAL_LINE_STATUS_THRE 0x20

static uint16_t serial_port =SERIAL_COM1_BASE;

static void serial_configure_baud_rate(uint16_t com,uint16_t divisor);
static void serial_configure_line(uint16_t com);
static void serial_configure_fifo(uint16_t com);
static void serial_configure_modem(uint16_t com);

static void serial_configure_baud_rate(uint16_t com,uint16_t divisor){
    //Enable DLAB so offset 0/1 become DLL/DLM
    outb(SERIAL_LINE_COMMAND_PORT(com),SERIAL_LINE_ENABLE_DLAB);

    //DLL=low byte,DLM=high byte
    outb(SERIAL_DATA_PORT(com),(uint8_t)(divisor & 0x00FF));
    outb(SERIAL_INTERRUPT_ENABLE(com),(uint8_t)((divisor>>8)& 0x00FF));
}

static void serial_configure_line(uint16_t com){
    // 8 data bits,no parity ,one stop bit, DLAB=0
    outb(SERIAL_LINE_COMMAND_PORT(com),SERIAL_LINE_8N1);
}

static void serial_configure_fifo(uint16_t com){
    outb(SERIAL_FIFO_COMMAND_PORT(com),SERIAL_FIFO_ENABLE_CLEAR_14);
}

static void serial_configure_modem(uint16_t com){
    outb(SERIAL_MODEM_COMMAND_PORT(com),SERIAL_MODEM_DTR_RTS);
}

void serial_initialize(void){
    //not using serial interrupts yet
    outb(SERIAL_INTERRUPT_ENABLE(serial_port),0x00);
    
    //divisor =3 -> 38400 baud when base clock is 115200
    serial_configure_baud_rate(serial_port,3);
    serial_configure_line(serial_port);
    serial_configure_fifo(serial_port);
    serial_configure_modem(serial_port);
}

int serial_is_transmit_fifo_empty(void) {
    return inb((SERIAL_LINE_STATUS_PORT(serial_port)) & SERIAL_LINE_STATUS_THRE)!=0;
}


void serial_putchar(char c){
    //common terminal-friendly choice: emit CR before LF
    if (c=='\n'){
        serial_putchar('\r');
    }

    while(!serial_is_transmit_fifo_empty()){
        //polling
    }

    outb(SERIAL_DATA_PORT(serial_port),(uint8_t)c);
}

int serial_write(const char *buf,size_t len){
    for(size_t i=0;i<len;i++){
        serial_putchar(buf[i]);
    }

    return (int)len;
}

void serial_writestring(const char *s){
    size_t len=0;
    while(s[len]!='\0'){
        len++;
    }
    serial_write(s,len);
}