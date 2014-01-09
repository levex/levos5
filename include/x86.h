#ifndef __X86_H_
#define __X86_H_

#include <stdint.h>

/* programmable interrupt controller methods */

extern int pic_init();
extern void pic_send_eoi(uint8_t irq);

/* global descriptor table methods */

extern int gdt_init();

/* interrupt descriptor table methods */
extern int idt_init();

/* programmable interval timer */
extern int pit_init();

/* paging methods */
extern int paging_init();

extern int exceptions_init();

struct trapframe {
	uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
	uint32_t esp;
	uint32_t ebp;
	uint32_t eip;
};

#define SAVE_REGISTERS_STACK asm volatile("pusha");
#define LOAD_REGISTERS_STACK asm volatile("popa");

#define SWITCH_THREAD(t) asm volatile("push %%eax; push %%ebx;" \
					"mov __new__thread, %%eax;" \
					"mov %%eax, __old__thread;" \
					"mov %0, %%eax;" \
					"mov %%eax, __new__thread;" \
					"pop %%ebx; pop %%eax;"::"m"(t)); \
					asm volatile("jmp switch_to_thread");

#define LOAD_STACK(new, old) asm volatile("mov %%esp, %0":"=m"(*old)); \
					asm volatile("mov %0, %%esp": :"m"(new));

#define INT_START asm volatile("pusha");
#define INT_END asm volatile("popa"); \
        asm volatile("iret");
        
#define IRQ_START asm volatile("add $0x1c, %esp"); \
                asm volatile("pusha");

#define IRQ_END asm volatile("popa"); \
        asm volatile("iret");

#endif
