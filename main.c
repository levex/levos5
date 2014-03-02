/* @author Levente Kurusa <levex@linux.com> */
#include <mm.h>
#include <tty.h>
#include <keyboard.h>
#include <hal.h>
#include <device.h>
#include <floppy.h>
#include <vfs.h>
#include <procfs.h>
#include <dummyfs.h>
#include <multiboot.h>
#include <syscall.h>
#include <elf.h>

#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wpointer-sign"


int root_mounted = 0;

int DISPLAY_ONLINE = 0;

void tty_watcher();
void __cursor_updater();

void main(struct multiboot *mb)
{
	int rc;
	
	rc = arch_early_init();
	if (rc)
		return;
		
	malloc(1);
	
	rc = device_core_init();
	if (rc)
		return;
	
	rc = tty_init(10);
	if (rc)
		return;
		
	rc = arch_late_init();
	if (rc)
		return;
		
	rc = keyboard_init();
	if (rc) {
		tty_write(2, (uint8_t *) "Keyboard failure!", 18);
		tty_flush(2);
		switch_to_tty(2);
	}
	DISPLAY_ONLINE = 1;
	switch_to_tty(0);

#ifdef _ARCH__arm__
	mailbox_init();
#endif
	
#ifdef CONFIG_ARCH_HAS_MULTIBOOT
	printk("Parsing multiboot headers...\n");
	if(mb->flags & 2)
		printk("Command line: %s\n", (char *)mb->cmdline);
	printk("mem_lower: 0x%x mem_upper: 0x%x\n", mb->mem_lower, mb->mem_upper);
	printk("mods: %d address: 0x%x\n", mb->mods_count, mb->mods_addr);
	
	phymem_init_level_two(mb->mem_upper * 1024);
	
	if(mb->mods_count > 1)
		panic("Ambigous module count!\n");
	printk("addr: 0x%x\n", read_eip());
	struct multiboot_mod *mod = (struct multiboot_mod *)mb->mods_addr;
	initrd_create((uint32_t *)mod->mod_start, (uint32_t *)mod->mod_end);
#else
	printk("Architecture has no multiboot headers. Memory set to 0xF00000.\n");
	phymem_init_level_two(0xF00000);
#endif
	
	scheduler_init();
	/*for(int i = 1; i < 11; i ++) 
		printk("malloc: 0x%x\n", malloc(0x1000));*/
		

	for(;;);
}


void test_lol()
{
	while(1) {
		tty_write(1, "1", 1);
		tty_flush(1);
	}
}

void test_lol2()
{
	while(1) {
		tty_write(2, "2", 1);
		tty_flush(2);
	}
}

uint8_t *sh_buf = 0;
uint32_t __esize = 0;
void __start_shell()
{
	char *const argv[] = {"hello", "world", 0};
	printk("Starting shell...\n");
	elf_start(sh_buf, __esize, argv, 0, 0);
	sys_exit(1);
	for(;;);
}

void start_shell()
{
	/* We need to start /bin/sh */
	//printk("sh_buf= 0x%x\n", sh_buf);
	struct stat st;
	struct file *fl = vfs_open("/bin/sh");
	if(!fl) goto err;
	sys_stat(fl->fullpath, &st);
	if(!st.st_size) goto err;
	sh_buf = malloc(st.st_size);
	if(!sh_buf) goto err;
	__esize = st.st_size;
	memset(sh_buf, 0, st.st_size);
	if(vfs_read_full(fl, sh_buf)) {
		struct process *p = create_new_process("shell", (uint32_t)__start_shell);
		struct file *f = vfs_open("/proc/devconf");
		vfs_write(f, "Hello DevConf 2014!\n", 20);
		if (!p)
			panic("Failed to create shell process!\n");
		int rc = scheduler_add_process(p);
		if (!rc)
			panic("Failed to add process!\n");
		//__start_shell();
		while(is_pid_running(rc)) schedule_noirq();
		panic("Attempted to kill init!\n");
	} else {
err:
		panic("Unable to start /bin/sh!");
	}
}


void late_init()
{
	printk("LevOS5.0, release date: %s (%s)\nFiring up drivers...\n", __DATE__, __TIME__);
	fdc_init();
	printk("Starting tty watcher\n");
	scheduler_add_process(create_new_process((uint8_t *)"tty_watcher", (uint32_t)tty_watcher));
	scheduler_add_process(create_new_process((uint8_t *)"cursor_update", (uint32_t)__cursor_updater));
	printk("Drivers done, setting up system calls\n");
	syscall_init();
	dummyfs_init();
	printk("Setting up vfs\n");
	if (vfs_init()) {
		panic("VFS failed to initialize!\n");
	}
	for(int i = 0; i < device_get_count(); i++) {
		struct device *dev = get_device(i);
		if(!dev)
			continue;
		
		if(dev->flags & DEVICE_FLAG_BLOCK) {
			if (device_try_to_mount(dev, "/")) {
				continue;
			} else {
				root_mounted = i;
				break;
			}
		}
	}
	if (!root_mounted) {
		panic("Unable to mount root directory\n");
	}
	if (device_try_to_mount(&procfs_dev, "/proc/")) {
		panic("Unable to mount procfs!\n");
	}
	//ext2_read_root_directory(0, get_device(root_mounted));
	list_mount();
	//START("test_lol", test_lol);
	START("test_lol2", test_lol2);
	start_shell();
	panic("Late init ended with no shell!\n");
	for(;;);
}

void __cursor_updater()
{
	uint16_t lastpos = 0;
	while(1)
	{
		struct display *disp = tty_get(tty_current())->disp;
		int x = 0, y = 0;
		disp->getpos(disp, &x, &y);
		uint16_t pos = y*80 + x;
		if(pos == lastpos) { schedule_noirq(); continue; }
		lastpos = pos;
		outportb(0x3D4, 0x0F);
		outportb(0x3D5, (uint8_t)pos & 0xFF);
		outportb(0x3D4, 0x0E);
		outportb(0x3D5, (uint8_t)(pos >> 8) & 0xFF);
		schedule_noirq();
	}
}

void tty_watcher()
{
	uint8_t *buf = malloc(64);
	memset(buf, 0, 64);
	while(1)
	{
		schedule_noirq();
		/*int read = tty_read(tty_current(), buf, 64);
		if(!read) { schedule_noirq(); continue; }
		if( buf[0] >= '0' && buf[0] <= '9' )
		{
			switch_to_tty(buf[0] - '0');
			schedule_noirq();
			continue;
		}
		tty_write(tty_current(), buf, read);
		tty_flush(tty_current());
		memset(buf, 0, 64);*/
	}
}
