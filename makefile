OBJS := main.o $(STARTFILE)

.PHONY: start all
all: start

ARCH=x86

include arch/$(ARCH)/makefile


include mm/makefile
include tty/makefile
include lib/makefile
include display/makefile
include drivers/makefile

kernel.img: $(OBJS)
	@echo "LD          $@"
	@$(LD) -T linker.ld -o kernel.img -ffreestanding -O2 -nostdlib $(OBJS)  -lgcc

start: kernel.img
	@echo "QEMU        $@"
	@$(QEMU) -kernel kernel.img

%.o: %.c
	@echo "CC          $@"
	@$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm $(OBJS)
