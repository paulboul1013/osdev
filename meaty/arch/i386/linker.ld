ENTRY(_start)

SECTIONS
{
    . = 1M;

    /* 必須第一個放，確保在檔案前 8KB 內讓 GRUB 能找到 */
    .multiboot :
    {
        *(.multiboot)
    }

    . = ALIGN(4K);
    .text :
    {
        *(.text)
        *(.rodata*)
    }

    . = ALIGN(4K);
    .data :
    {
        *(.data)
    }

    . = ALIGN(4K);
    .bss :
    {
        *(COMMON)
        *(.bss)
    }
}