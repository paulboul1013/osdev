.global gdt_flush
.type gdt_flush, @function

gdt_flush:
    mov 4(%esp), %eax #eax=&gdt_ptr
    lgdt (%eax)

    mov $0x10, %ax # data selector =entry 2<<3 =0x10
    mov %ax,%ds
    mov %ax,%es
    mov %ax,%fs
    mov %ax,%gs
    mov %ax,%ss

    ljmp $0x08, $flush_done # code selector = entry 1<<3 =0x08

flush_done:
    ret