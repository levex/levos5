/** @author Levente Kurusa <levex@linux.com> **/
#include <tty.h>
#include <hal.h>
#include <string.h>
#include <stdint.h>

#define kprintf(x) tty_write(tty_current(), (uint8_t *)x, strlen((uint8_t *)x)); \
			tty_flush(tty_current());

void exc_divide_by_zero()
{
	kprintf("Divide by zero!\n");
	for(;;);
}

void exc_debug()
{
	kprintf("Debug!\n");
	for(;;);
}

void exc_nmi()
{
	kprintf("NMI\n");
	for(;;);
	return;
}

void exc_brp()
{
	kprintf("Breakpoint!\n");
	return;
}

void exc_overflow()
{
	kprintf("Overflow!\n");
	for(;;);
}

void exc_bound()
{
	kprintf("Bound range exceeded.\n");
	for(;;);
}

void exc_invopcode()
{
	kprintf("Invalid opcode.\n");
	for(;;);
}

void exc_device_not_avail()
{
	kprintf("Device not available.\n");
	for(;;);
}

void exc_double_fault()
{
	kprintf("Double fault, halting.\n");
	for(;;);
}

void exc_coproc()
{
	kprintf("Coprocessor fault, halting.\n");
	for(;;);
	return;
}

void exc_invtss()
{
	kprintf("TSS invalid.\n");
	for(;;);
	return;
}

void exc_segment_not_present()
{
	kprintf("Segment not present.\n");
	for(;;);
	return;
}

void exc_ssf()
{
	kprintf("Stacksegment faulted.\n");
	for(;;);
	return;
}

void exc_gpf()
{
	printk("General protection fault in process %s pid: %d\n", get_process()->namebuf, get_process()->pid);
	scheduler_kill_self();
	return;
}

void exc_pf()
{
	uint32_t cr2 = 0;
	asm volatile("mov %%cr2, %%eax":"=a"(cr2));
	printk("Page fault! [cr2: 0x%x] PROCESS: %s pid: %d\n", cr2, get_process()->namebuf, get_process()->pid);
	scheduler_kill_self();
}

void exc_reserved()
{
	kprintf("This shouldn't happen. Halted.\n");
	for(;;);
}

int exceptions_init()
{
	register_interrupt(0, exc_divide_by_zero);
	register_interrupt(1, exc_debug);
	register_interrupt(2, exc_nmi);
	register_interrupt(3, exc_brp);
	register_interrupt(4, exc_overflow);
	register_interrupt(5, exc_bound);
	register_interrupt(6, exc_invopcode);
	register_interrupt(7, exc_device_not_avail);
	register_interrupt(8, exc_double_fault);
	register_interrupt(9, exc_coproc);
	register_interrupt(10, exc_invtss);
	register_interrupt(11, exc_segment_not_present);
	register_interrupt(12, exc_ssf);
	register_interrupt(13, exc_gpf);
	register_interrupt(14, exc_pf);
	register_interrupt(15, exc_reserved);
	return 0;
}
