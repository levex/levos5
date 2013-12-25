#ifndef __HAL_H_
#define __HAL_H_

#include <stdint.h>
#include <input.h>
#include <display.h>

#ifdef _ARCH__x86__
	#include <x86.h>
#endif

#ifdef _ARCH__arm__
	#include <arm.h>
#endif

/* arch-specific functions */

/* early init for arch-spec tables and such */
extern int arch_early_init();
/* late init. should register a ticker with the scheduler */
extern int arch_late_init();

/* register an interrupt (or such) handler */
extern int register_interrupt(int no, void(*func)());
/* send EndOfInterrupt to IRQ #no */
extern int send_eoi(int no);
/* enable(1) or disable(0) interrupts */
extern int interrupt_ctl(int enable);

/* return a pointer to a new default display for a tty */
extern struct display *arch_new_default_display(struct tty *mtty);

/* return a pointer to a new default input for a tty */
extern struct input *arch_new_default_input(struct tty *mtty);

/* return a pointer to #pages allocation */
extern void *mm_alloc_pages(int pages);

/* free those */
extern int mm_free_pages(void *ptr, int pages);

/* inport, outport */
uint8_t inportb(uint16_t portid);
uint16_t inportw(uint16_t portid);
uint32_t inportl(uint16_t portid);
void outportb(uint16_t portid, uint8_t value);
void outportw(uint16_t portid, uint16_t value);
void outportl(uint16_t portid, uint32_t value);

/* end of arch-spec functions */

extern uint8_t inportb(uint16_t portid);
extern uint16_t inportw(uint16_t portid);
extern uint32_t inportl(uint16_t portid);
extern void outportb(uint16_t portid, uint8_t value);
extern void outportw(uint16_t portid, uint16_t value);
extern void outportl(uint16_t portid, uint32_t value);

#endif
