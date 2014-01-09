#include <hal.h>
#include <stdint.h>
#include <x86.h>
#include <tty.h>
#include <textmode.h>
#include <display.h>
#include <input.h>
#include <keyboard.h>

int arch_early_init()
{
	int rc;
	/* init GDT */
	rc = gdt_init();
	if (rc)
		return rc;
	/* init IDT */
	rc = idt_init();
	if (rc)
		return rc;
	/* init PIC */
	rc = pic_init();
	if (rc)
		return rc;
	/* init PIT */
	rc = pit_init();
	if (rc)
		return rc;

	rc = exceptions_init();
	if (rc)
		return rc;

	/* init paging */
	rc = paging_init();
	
	return rc;
}

int arch_late_init()
{
	tty_write(0, (uint8_t *)"x86 architecture init succeeded\n", 32);
	tty_flush(0);
	
	/* init tasking */
	return 0;
}

int interrupt_ctl(int e)
{
	if(e)
		asm volatile("sti");
	else
		asm volatile("cli");
		
	return 0;
}

/* return a pointer to a new default display for a tty */
struct display *arch_new_default_display(struct tty *mtty)
{
	return textmode_display_new(mtty);
}

/* return a pointer to a new default input for a tty */
struct input *arch_new_default_input(struct tty *mtty)
{
	return keyboard_create_new(mtty);
}
