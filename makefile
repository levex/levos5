OBJS := main.o $(STARTFILE)

.PHONY: start all mount
all: start

ARCH=x86

include arch/$(ARCH)/makefile


include mm/makefile
include tty/makefile
include lib/makefile
include display/makefile
include drivers/makefile
include kernel/makefile
include fs/makefile
include net/makefile

kernel.img: $(OBJS)
	@echo "  LD          $@"
	@$(LD) -T linker.ld.$(ARCH) -o kernel.img $(LDFLAGS) $(OBJS)  -lgcc

start: kernel.img
	@echo "  DEBUGSYM    $<"
	@$(OBJCOPY) --only-keep-debug kernel.img kernel.sym
	@echo "  QEMU        $<"
	@$(QEMU) -s -initrd fda.img -kernel kernel.img -net nic,model=ne2k_pci -net user -net dump,file=/dev/stdout -serial stdio

%.o: %.c
	@echo "  CC          $@"
	@$(CC) -c -g $(CFLAGS) $< -o $@

clean:
	-rm $(OBJS)

mount:
	sudo mount fda.img /mnt/fda



