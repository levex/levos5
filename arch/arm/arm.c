#include <arm.h>
#include <hal.h>
#include <stdint.h>

/* arch-specific functions */

/* early init for arch-spec tables and such */
int arch_early_init()
{
	return 0;
}
/* late init. should register a ticker with the scheduler */
int arch_late_init()
{
	tty_write(0, (uint8_t *)"hello arm world!", 17);
	tty_flush(0);
	return 0;
}

/* register an interrupt (or such) handler */
int register_interrupt(int no, void(*func)())
{
	no = no;
	func = func;
	return 0;
}
/* send EndOfInterrupt to IRQ #no */
int send_eoi(int no)
{
	no = no;
	return 0;
}
/* enable(1) or disable(0) interrupts */
int interrupt_ctl(int enable)
{
	enable = enable;
	return 0;
}

/* return a pointer to a new default display for a tty */
struct display *arch_new_default_display(struct tty *mtty)
{
	return pl110_create_new(mtty);
}

/* return a pointer to a new default input for a tty */
struct input *arch_new_default_input(struct tty *mtty)
{
	mtty = mtty;
	return 0;
}

uint32_t *last_alloc = (uint32_t *)0x4000000;

/* return a pointer to #pages allocation */
void *mm_alloc_pages(int pages)
{
	void *ret = last_alloc;
	last_alloc += 0x1000 * pages;
	return ret;
}

/* free those */
int mm_free_pages(void *ptr, int pages)
{
	ptr = ptr;
	pages = pages;
	return 0;
}

uint8_t inportb(uint16_t portid)
{
	portid = portid;
	return 0;
}
uint16_t inportw(uint16_t portid)
{
	portid = portid;
	return 0;
}
uint32_t inportl(uint16_t portid)
{
	portid = portid;
	return 0;
}
void outportb(uint16_t portid, uint8_t value)
{
	portid = portid;
	value = value;
}
void outportw(uint16_t portid, uint16_t value)
{
	portid = portid;
	value = value;
}
void outportl(uint16_t portid, uint32_t value)
{
	portid = portid;
	value = value;
}

/* end of arch-spec functions */
