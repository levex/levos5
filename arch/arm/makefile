CC=arm-elf-gcc
AS=arm-elf-as
LD=arm-elf-gcc
QEMU=qemu-system-arm
CFLAGS=-D_ARCH__$(ARCH)__ -Iinclude -std=gnu99 -ffreestanding -O2 -Wall -Wextra
OBJCOPY=arm-elf-objcopy

OBJS += arch/arm/start.o
OBJS += arch/arm/arm.o
OBJS += arch/arm/sched.o

include arch/arm/drivers/makefile

.PHONY += rpi
rpi:
	~/qemu-rpi/bin/qemu-system-arm -kernel kernel.img -M raspi -m 256M -serial stdio

STARTFILE := arch/arm/start.o
arch/arm/start.o: arch/arm/start.s
	@echo "AS          $@"
	@$(AS) -c $< -o $@


