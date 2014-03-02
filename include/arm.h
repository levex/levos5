#ifndef __ARM_H_
#define __ARM_H_

#include <tty.h>

#define INT_START ;
#define INT_END ;
        
#define IRQ_START ;

#define IRQ_END ;

#define SAVE_REGISTERS_STACK asm volatile("PUSH {r0-r12}");
#define LOAD_REGISTERS_STACK asm volatile("POP {r0-r12}");

#define ARCH_SAVE_PAGED(x) ;
#define START_EXECUTION_BY_JUMPING(x, argv, argc) ;

#if 0
	#define SWITCH_THREAD(t) asm volatile("push %%eax; push %%ebx;" \
						"mov __new__thread, %%eax;" \
						"mov %%eax, __old__thread;" \
						"mov %0, %%eax;" \
						"mov %%eax, __new__thread;" \
						"pop %%ebx; pop %%eax;"::"m"(t)); \
						asm volatile("jmp switch_to_thread");
#endif

#define SWITCH_THREAD(x) SAVE_REGISTERS_STACK; \
						__old__thread = __new__thread; \
						__new__thread = x; \
						LOAD_REGISTERS_STACK; \
						asm volatile("b switch_to_thread");
						
						
/* save address of __old__thread to r2 */
/* move new_thread to r0 */
/* load r0 (new_thread) to the value pointed by r2 */

extern void __arm__put32(uint32_t a, uint32_t b);
extern uint32_t __arm__get32(uint32_t a);

extern struct display *pl110_create_new(struct tty *mtty);

#endif
