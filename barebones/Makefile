override PREFIX := $(HOME)/osdev/opt/cross/bin/i686-elf
CC      := $(PREFIX)-gcc
AS      := $(PREFIX)-as
CFLAGS  := -std=gnu11 -ffreestanding -O2 -Wall -Wextra
LDFLAGS := -T linker.ld -ffreestanding -O2 -nostdlib

all: myos.iso

boot.o: boot.s
	$(AS) boot.s -o boot.o

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

mykernel.bin: boot.o kernel.o linker.ld
	$(CC) $(LDFLAGS) boot.o kernel.o -lgcc -o mykernel.bin

myos.iso: mykernel.bin grub.cfg
	mkdir -p iso/boot/grub
	cp mykernel.bin iso/boot/mykernel.bin
	cp grub.cfg iso/boot/grub/grub.cfg
	grub-mkrescue -o myos.iso iso

run: myos.iso
	qemu-system-i386 -cdrom myos.iso

clean:
	rm -f *.o *.bin *.iso
	rm -rf iso/boot/mykernel.bin iso/boot/grub/grub.cfg