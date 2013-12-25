CC=arm-elf-gcc
AS=arm-elf-as
LD=arm-elf-gcc
QEMU=qemu-system-arm
CFLAGS=-D_ARCH__$(ARCH)__ -Iinclude -std=gnu99 -ffreestanding -O2 -Wall -Wextra

OBJS += arch/arm/start.o
OBJS += arch/arm/arm.o

include arch/arm/drivers/makefile

STARTFILE := arch/arm/start.o
arch/arm/start.o: arch/arm/start.s
	@echo "AS          $@"
	@$(AS) -c $< -o $@


