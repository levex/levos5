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

#define INT_START asm volatile("pusha");
#define INT_END asm volatile("popa"); \
        asm volatile("iret");
        
#define IRQ_START asm volatile("add $0x1c, %esp"); \
                asm volatile("pusha");

#define IRQ_END asm volatile("popa"); \
        asm volatile("iret");

#endif
