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
include kernel/makefile

kernel.img: $(OBJS)
	@echo "LD          $@"
	@$(LD) -T linker.ld -o kernel.img -ffreestanding -O2 -nostdlib $(OBJS)  -lgcc

start: kernel.img
	@echo "DEBUGSYM    $<"
	@objcopy --only-keep-debug kernel.img kernel.sym
	@echo "QEMU        $<"
	@$(QEMU) -s -kernel kernel.img

%.o: %.c
	@echo "CC          $@"
	@$(CC) -c -g $(CFLAGS) $< -o $@

clean:
	rm $(OBJS)
