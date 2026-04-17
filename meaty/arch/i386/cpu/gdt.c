#include <stdint.h>
#include "../gdt.h"

static struct gdt_entry gdt[3];
static struct gdt_ptr gp;

static void gdt_set_gate(int num,uint32_t base,uint32_t limit,uint8_t access,uint8_t gran){
    gdt[num].base_low=(base&0xFFFF);
    gdt[num].base_middle=(base>>16)&0xFF;
    gdt[num].base_high=(base>>24)&0xFF;

    //20 bits limit
    gdt[num].limit_low=(limit&0xFFFF);
    gdt[num].granularity=(limit>>16)&0x0F;
    
    gdt[num].granularity|=gran&0xF0;
    gdt[num].access=access;
}

void gdt_init(void){
    gp.limit=sizeof(gdt)-1;
    gp.base=(uint32_t)&gdt;

    // 0: null descriptor
    gdt_set_gate(0,0,0,0,0);

    //1: kernel code segement
    gdt_set_gate(1,0,0xFFFFFFFF,0x9A,0xCF);

    //2: kernel data segement
    gdt_set_gate(2,0,0xFFFFFFFF,0x92,0xCF);

    gdt_flush((uint32_t)&gp);
}