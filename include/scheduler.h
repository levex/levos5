#ifndef __SCHEDULER_H_
#define __SCHEDULER_H_

#define MAX_THREADS 16
#define MAX_PROCESSES 32

#define THREAD_STACK_SIZE 4096
#include <hal.h>
struct trapframe;

struct thread {
	uint32_t ip; /* instruction pointer for the next instruction */
	uint32_t stack; /* stack pointer */
	struct trapframe *frame;
};

struct process {
	uint8_t *namebuf; /* ZTS containing the name of the process*/
	uint8_t lastthread; /* id of the last thread that was executed */
	uint8_t threadslen; /* number of threads in @threads */
	struct thread *threads[MAX_THREADS]; /* array of MAX_THREADS pointers to the threads */
};

extern int scheduler_ctl(int enable);
extern void scheduler_switch();
extern int scheduler_add_process(struct process *p);
extern struct process *create_new_process(uint8_t *namebuf, uint32_t addr);
extern int create_new_thread(struct process *p, uint32_t s);
extern void scheduler_init();

#endif
