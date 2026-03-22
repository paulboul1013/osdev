.set ALIGN, 1<<0 #載入模組時要做對齊（alignment）
.set MEMINFO, 1<<1 # bootloader 提供記憶體資訊
.set FLAGS, ALIGN | MEMINFO
.set MAGIC, 0x1BADB002 # Multiboot 規範要求的 magic number
.set CHECKSUM, -(MAGIC+FLAGS) # MAGIC + FLAGS + CHECKSUM == 0   （32-bit unsigned 加總）

.section .multiboot # 目前位置對齊到 4 bytes 邊界，放的是 .long（4 bytes），對齊可以避免格式不正確
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .text
.global _start # _start 宣告成全域符號，linker 和 bootloader 才能知道這是入口符號之一
.type _start, @function # 告訴 assembler / linker _start 這個 symbol 是一個函式
_start:
    mov $stack_top, %esp # 把 stack_top 的位址放進 esp。
    call kernel_main
1:  cli # 關閉可屏蔽中斷
    hlt
    jmp 1b # 跳回前面的 1: 標籤

.section .bss
.align 16
stack_bottom:
.skip 16384 # 保留  16 KiB 的空間。拿來當 stack 用
stack_top:



