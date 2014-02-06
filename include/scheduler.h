#ifndef __SCHEDULER_H_
#define __SCHEDULER_H_

#define MAX_THREADS 16
#define MAX_PROCESSES 32
#define MAX_FILEHANDLES 128

#define THREAD_STACK_SIZE 4096
#include <hal.h>
#include <vfs.h>
struct trapframe;

struct thread {
	uint32_t ip; /* instruction pointer for the next instruction */
	volatile uint32_t stack; /* stack pointer */
	volatile uint32_t stacktop;
	volatile uint32_t *stackbot;
	uint32_t *paged; /* points to the processes' paged filed */
	struct trapframe *frame;
};

struct process {
	uint8_t *namebuf; /* ZTS containing the name of the process*/
	uint8_t lastthread; /* id of the last thread that was executed */
	uint8_t threadslen; /* number of threads in @threads */
	uint8_t tty_id; /* the tty the process is connected to */
	uint32_t paged; /* pagedirectory address */
	uint8_t pid; /* process' id */
	uint32_t palloc;
	uint32_t palloc_len;
	uint32_t exec_len;
	uint32_t open_handles;
	struct file **filehandles;
	struct thread *threads[MAX_THREADS]; /* array of MAX_THREADS pointers to the threads */
};

extern struct process *get_process();
extern int is_pid_running(int pid);
extern int scheduler_ctl(int enable);
extern void scheduler_switch();
extern int scheduler_add_process(struct process *p);
extern struct process *create_new_process(uint8_t *namebuf, uint32_t addr);
extern struct process *create_new_process_nothread(uint8_t *namebuf);
extern int create_new_thread(struct process *p, uint32_t s, int a);
extern void scheduler_init();

extern int scheduler_kill_self();

#define START(name, entry) scheduler_add_process(create_new_process(name, entry));

#endif
