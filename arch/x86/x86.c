#include <hal.h>
#include <stdint.h>
#include <x86.h>
#include <tty.h>
#include <textmode.h>
#include <display.h>
#include <input.h>
#include <keyboard.h>

extern uint32_t kernel_stack;

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

int task = 0;

void enable_scheduling()
{
	task = 1;
	return;
}

static uint32_t __esp__esp_ = 0;
void load_stack(uint32_t new, uint32_t *old)
{
	asm volatile("mov %%esp, %%eax":"=a"(__esp__esp_));
	*old = __esp__esp_;
	asm volatile("mov %%eax, %%esp": :"a"(new));
}

/* defined by the scheduler */
extern struct thread *__old__thread;
extern struct thread *__new__thread;
/* Switch context to thread */
void switch_to_thread()
{
	asm volatile("cmp $0, __old__thread");
	asm goto("je %l0"::::__no__old__thread);
		asm volatile("push %eax");
		asm volatile("push %ebx");
		asm volatile("push %ecx");
		asm volatile("push %edx");
		asm volatile("push %esi");
		asm volatile("push %edi");
		asm volatile("push %ebp");
		asm volatile("push %ds");
		asm volatile("push %es");
		asm volatile("push %fs");
		asm volatile("push %gs");
		/* save stack */
		asm volatile("mov %%esp, %%eax": "=a"(__old__thread->stack));
__no__old__thread:
	/* load stack */
	asm volatile("mov %%eax, %%esp"::"a"(__new__thread->stack));
	asm volatile("pop %gs");
	asm volatile("pop %fs");
	asm volatile("pop %es");
	asm volatile("pop %ds");
	asm volatile("pop %ebp");
	asm volatile("pop %edi");
	asm volatile("pop %esi");
	/* send End Of Interrupt to Master PIC */
	asm volatile("out %%al, %%dx": :"d"(0x20), "a"(0x20));
	asm volatile("pop %edx");
	asm volatile("pop %ecx");
	asm volatile("pop %ebx");
	asm volatile("pop %eax");
	asm volatile("iret");
}
