#include <arm.h>
#include <scheduler.h>


void schedule_noirq()
{
	;
}

void arch_setup_paged(uint32_t *out, uint32_t palloc)
{
	;
}

void syscall_init()
{
	;
}

uint32_t read_eip()
{
	return 0;
}

void kbd_irq()
{
	;
}

void enable_scheduling()
{
	;
}
extern struct thread *__new__thread;
void switch_to_thread()
{
	void (*ptr)(void);
	ptr = (void (*)(void))__new__thread->ip;
	ptr();
}
