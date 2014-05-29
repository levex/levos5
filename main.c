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
#include <dbus.h>
#include <errno.h>

#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wpointer-sign"


int root_mounted = 0;

int DISPLAY_ONLINE = 0;


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


uint8_t *sh_buf = 0;
uint32_t __esize = 0;
void __start_shell()
{
	char *const argv[] = {"hello", "world", 0};
	elf_start(sh_buf, __esize, argv, 0, 0);
	sys_exit(1);
	for(;;);
}

int init_killed_event(struct dbus_message *msg)
{
	if (msg->msg != DBUS_EVENT_INIT_KILLED)
		return -EINVAL;

	panic("Attempted to kill init!\n");
	return 0;
}

struct dbus_listener init_kill_listener = {
	.msg = DBUS_EVENT_INIT_KILLED,
	.type = DBUS_LISTEN_TYPE_ONESHOT,
	.callback = init_killed_event,	
};

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
		dbus_register_listener(&init_kill_listener);
	} else {
err:
		panic("Unable to start /bin/sh!");
	}
}


void late_init()
{
	printk("LevOS5.0, release date: %s (%s)\nFiring up drivers...\n", __DATE__, __TIME__);
	fdc_init();
	syscall_init();
	dummyfs_init();
	dbus_init();
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
	pci_init();
	ne2k_init();
	list_mount();
	start_shell();
	for(;;) schedule_noirq();
}

