#include <hal.h>
#include <stdint.h>
#include <x86.h>
#include <tty.h>
#include <textmode.h>
#include <display.h>
#include <input.h>
#include <keyboard.h>
#include <syscall.h>
#include <vfs.h>

extern uint32_t kernel_stack;

int arch_early_init()
{
	int rc;
	
	init_serial();
	
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

void __imp_sys_dispatch()
{
	uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
	asm volatile("xchg %%eax, %%eax":"=a"(eax));
	asm volatile("xchg %%ebx, %%ebx":"=b"(ebx));
	asm volatile("xchg %%ecx, %%ecx":"=c"(ecx));
	asm volatile("xchg %%edx, %%edx":"=d"(edx));
	//if(eax > 0xdf)
		//printk("SYSCALL: 0x%x 0x%x 0x%x 0x%x\n", eax, ebx, ecx, edx);
	switch(eax) {
		case 0x1: {
			sys_exit(ebx);
			return;
		}
		case 0x2: {
			asm volatile("mov %0, %%eax"::"a"(sys_fork()));
			return;
		}
		case 0x3: {
			asm volatile("mov %0, %%eax"::"a"(sys_read(ebx, (uint8_t *)ecx, edx)));
			return;
		}
		case 0x4: {
			sys_write(ebx, (uint8_t *)ecx, edx);
			return;
		}
		case 0x5: {
			asm volatile("mov %0, %%eax"::"a"(sys_open((char *)ebx, (int)ecx)));
			return;
		}
		case 0xB: {
			asm volatile("mov %0, %%eax"::"a"(sys_execve((char *)ebx, (char **)ecx, (char **)edx)));
			return;
		}
		case 0x12: {
			sys_stat((char *)ebx, (struct stat *)ecx);
			return;
		}
		case 0x14: {
			asm volatile("mov %0, %%eax"::"a"(sys_getpid()));
			return;
		}
		case 0xe0: {
			sys_uname((struct utsname *)ebx);
			return;
		}
		case 0xed: {
			asm volatile("mov %0, %%eax"::"a"(sys_opendir((char *)ebx)));
			return;
		}
		case 0xee: {
			asm volatile("mov %0, %%eax"::"a"(sys_readdir(ebx)));
			return;
		}
		case 0xef: {
			sys_waitpid(ebx);
			return;
		}
	}
	return;
}

void syscall_init()
{
	register_interrupt(0x80, sys_dispatch);
}

int __irq_enabled = 0;

int interrupts_enabled()
{
	return __irq_enabled;
}

int interrupt_ctl(int e)
{
	if(e) {
		__irq_enabled = 1;
		asm volatile("sti");
	} else {
		__irq_enabled = 0;
		asm volatile("cli");
	}
		
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

extern int init_ready;
void schedule_noirq()
{
	if (init_ready && task && interrupts_enabled() )
		asm volatile("int $0x2f");
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
	asm volatile("mov %%eax, %%cr3"::"a"(*__new__thread->paged));
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
