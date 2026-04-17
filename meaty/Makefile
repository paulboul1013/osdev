override PREFIX := $(HOME)/osdev/opt/cross/bin/i686-elf
CC      := $(PREFIX)-gcc
AS      := $(PREFIX)-as
CFLAGS  := -std=gnu11 -ffreestanding -O2 -Wall -Wextra
LDFLAGS := -T arch/i386/linker.ld -ffreestanding -O2 -nostdlib

OBJS := arch/i386/boot.o kernel/kernel.o kernel/tty.o kernel/serial.o

all: myos.iso

arch/i386/boot.o: arch/i386/boot.s
	$(AS) $< -o $@

kernel/kernel.o: kernel/kernel.c
	$(CC) $(CFLAGS) -c $< -o $@

kernel/tty.o: kernel/tty.c
	$(CC) $(CFLAGS) -c $< -o $@

kernel/serial.o: kernel/serial.c
	$(CC) $(CFLAGS) -c $< -o $@

mykernel.bin: $(OBJS) arch/i386/linker.ld
	$(CC) $(LDFLAGS) $(OBJS) -lgcc -o $@

myos.iso: mykernel.bin iso/boot/grub/grub.cfg
	cp mykernel.bin iso/boot/mykernel.bin
	grub-mkrescue -o $@ iso

run: myos.iso
	qemu-system-i386 -cdrom myos.iso -serial mon:stdio

run-serial: myos.iso
	qemu-system-i386 -cdrom myos.iso -nographic

debug: myos.iso
	qemu-system-i386 -cdrom myos.iso -s -S

clean:
	rm -f arch/i386/*.o kernel/*.o mykernel.bin myos.iso