OBJS := main.o start.o

ARCH=x86

include arch/$(ARCH)/makefile

.PHONY: start all

all: start

start.o: start.s
	@$(AS) -c $< -o $@
	@echo AS $@

include mm/makefile
include tty/makefile
include lib/makefile
include display/makefile
include drivers/makefile

kernel.img: $(OBJS)
	@i586-elf-gcc -T linker.ld -o kernel.img -ffreestanding -O2 -nostdlib $(OBJS)  -lgcc
	@echo LD $@

start: kernel.img
	qemu-system-i386 -kernel kernel.img

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@
	@echo CC $@

clean:
	rm $(OBJS)
