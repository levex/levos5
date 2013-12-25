CC=i586-elf-gcc
AS=i586-elf-as
CFLAGS=-D_ARCH__$(ARCH)__ -Iinclude -std=gnu99 -ffreestanding -O2 -Wall -Wextra

OBJS += arch/x86/hal.o
OBJS += arch/x86/x86.o
OBJS += arch/x86/pic.o
OBJS += arch/x86/gdt.o
OBJS += arch/x86/idt.o
OBJS += arch/x86/pit.o
OBJS += arch/x86/paging.o
