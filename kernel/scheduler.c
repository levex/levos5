#include <hal.h>
#include <timer.h>
#include <mm.h>
#include <scheduler.h>
#include <string.h>

#pragma GCC diagnostic ignored "-Warray-bounds"

static uint8_t scheduler_status = 0;

static uint8_t last_process = 0;
struct process *processes[32];
struct thread *mt = 0;
static uint8_t __lastpid = 0;
struct thread *__old__thread = 0;
struct thread *__new__thread = 0;

/* two variables to be used by the $arch */
uint32_t kernel_stack = 0;
uint32_t user_stack = 0;

static struct thread *__t = 0;
static struct process *__p = 0;

extern void late_init();

void idle_task()
{
	tty_write(tty_current(), (uint8_t *)"Tasking online.\n", 16);
	tty_flush(tty_current());
	scheduler_add_process(create_new_process((uint8_t *)"late_init", (uint32_t)late_init));
	enable_scheduling();
	while(1) schedule_noirq();
	for(;;) schedule_noirq();
}

/**
 * create_new_thread - Create a new thread for the @p process
 * 			with starting instruction at @s. 
 *
 * @p - struct process to add the thread to.
 * @s - address of the instruction that is the first to be executed
 * @a - if 1, then we need mutual exclusion!
 * 
 * \returns 0 on success, 1 on failure
 */ 
int create_new_thread(struct process *p, uint32_t s, int a)
{
	int rc = 0;
	
	/* first, switch off scheduler to avoid broken processes */
	if(a)
		interrupt_ctl(0);
	
	/* check if the process actually exists */
	if(!p || !s)
		goto err;
	
	/* Check if the process can have an extra thread */
	if(p->threadslen + 1 > MAX_THREADS)
		goto err;
	
	/* allocate space for the thread */
	struct thread *t = malloc(sizeof(struct thread));
	if(!t)
		goto err;
	memset(t, 0, sizeof(struct thread));
		
	/* set the thread's entry point */
	t->ip = s;

	/* allocate space for the thread's stack */
	volatile uint32_t *stack = malloc(THREAD_STACK_SIZE);
	if(!stack)
		goto err_s;
	t->stackbot = stack;
	memset(stack, 0, THREAD_STACK_SIZE);
	stack = (volatile uint32_t *)(((uint32_t)stack + 4000 )& 0xfffffff0);

	volatile uint32_t stacktop = (uint32_t) stack;
	t->stacktop = stacktop;
		
	/* setup the stack of the thread */
	*--stack = 0x00000202; // eflags
	*--stack = 0x8; // cs
	*--stack = (uint32_t)s; // eip
	*--stack = 0; // eax
	*--stack = 0; // ebx
	*--stack = 0; // ecx;
	*--stack = 0; //edx
	*--stack = 0; //esi
	*--stack = 0; //edi
	*--stack = stacktop; //ebp
	*--stack = 0x10; // ds
	*--stack = 0x10; // fs
	*--stack = 0x10; // es
	*--stack = 0x10; // gs

	t->stack = (volatile uint32_t)stack;
	
	t->paged = &p->paged;

	/* put the thread into the process */
	p->threads[p->threadslen] = t;

	
	/* increment the number of threads controlled by the process */
	p->threadslen ++;
	
	if(a)
		interrupt_ctl(1);

	
	return 0;
err_s:
	free(t);
err:
	rc = 1;
	if(a)
		interrupt_ctl(1);
	return rc;
}

/**
 * create_new_process_nothread - Create a new process without a thread
 * 
 * @namebuf - A ZTS that contains the name of the process
 * 
 * \note This function does NOT add the process to the queue!
 * \returns the process created or 0 on failure
 */
struct process *create_new_process_nothread(uint8_t *namebuf)
{
	/* 
	* Because we don't actually tinker with running data, we don't
	* switch off the scheduler
	*/

	int rc = 0;
	interrupt_ctl(0);

	/* allocate space for the process */
	struct process *p = malloc(sizeof(struct process));
	if(!p)
		goto err;
	memset(p, 0, sizeof(struct process));

	/* allocate space for the name */
	uint8_t *name = malloc(strlen(namebuf) + 1);
	if(!name)
		goto err_2;

	/* copy the name over */
	memcpy(name, namebuf, strlen(namebuf) + 1);

	/* set up the process */
	p->namebuf = name;	
	p->lastthread = 0;
	p->threadslen = 0;
	p->tty_id = 0;
	asm volatile("mov %%cr3, %%eax":"=a"(p->paged));
	/* set the pid */
	p->pid = __lastpid;
	__lastpid ++;
		
	interrupt_ctl(1);
	return p;

err_3:
	free(name);
err_2:
	free(p);
err:
	interrupt_ctl(1);
	return 0;
}

/**
 * create_new_process - Create a new process
 * 
 * @namebuf - A ZTS that contains the name of the process
 * @addr - An address at which the first thread will begin execution
 * 
 * \note This function does NOT add the process to the queue!
 * \returns the process created or 0 on failure
 */
struct process *create_new_process(uint8_t *namebuf, uint32_t addr)
{
	/* 
	* Because we don't actually tinker with running data, we don't
	* switch off the scheduler
	*/

	int rc = 0;
	interrupt_ctl(0);

	/* allocate space for the process */
	struct process *p = malloc(sizeof(struct process));
	if(!p)
		goto err;
	memset(p, 0, sizeof(struct process));

	/* allocate space for the name */
	uint8_t *name = malloc(strlen(namebuf) + 1);
	if(!name)
		goto err_2;

	/* copy the name over */
	memcpy(name, namebuf, strlen(namebuf) + 1);

	/* set up the process */
	p->namebuf = name;	
	p->lastthread = 0;
	p->threadslen = 0;
	p->tty_id = 0;
	asm volatile("mov %%cr3, %%eax":"=a"(p->paged));
	/* set the pid */
	p->pid = __lastpid;
	__lastpid ++;

	/* and now create a new thread in the process */
	rc = create_new_thread(p, addr, 0);
	if(rc)
		goto err_3;
		
	interrupt_ctl(1);
	return p;

err_3:
	free(name);
err_2:
	free(p);
err:
	interrupt_ctl(1);
	return 0;
}

int is_pid_running(int pid)
{
	for(int i = 0; i < MAX_PROCESSES; i++) {
		if(!processes[i])
			continue;

		if( processes[i]->pid == pid ) {
			return 1;
		}
	}
	return 0;
}

/**
 * scheduler_kill_self - Kill currently running process
 * 
 * NORETURN!
 */
int scheduler_kill_self()
{
	interrupt_ctl(0);
	/* remove old thread from scheduler */
	__old__thread = 0;
	/* find current process in hashtable */
	for(int i = 0; i < MAX_PROCESSES; i++) {
		if(!processes[i])
			continue;

		if( processes[i] == __p ) {
			processes[i] = 0;
			break;
		}
	}
	/* free the stuff registered with it */
	free(__p->namebuf);
	phymem_free(__p->palloc, __p->palloc_len);
	free(__p->threads[0]->stackbot);
	free(__p);
	/* @TODO */
	interrupt_ctl(1);
	/* now schedule away from this */
	schedule_noirq();
	/* doesn't actually return, but need to silence gcc */
	return 1;
}

/**
 * scheduler_kill_pid - Kill a process with given PID
 * 
 * @pid - The process' id, which needs to be killed
 * \returns 0 on success, 1 on failure
 */
int scheduler_kill_pid(int pid)
{
	int killed = 0;
	interrupt_ctl(0);
	
	if(pid == __p->pid)
		return scheduler_kill_self();
	
	for(int i = 0; i < MAX_PROCESSES; i++) {
		if(!processes[i])
			continue;

		if( processes[i]->pid == pid ) {
			processes[i] = 0;
			killed = 1;
		}
	}
	interrupt_ctl(1);
	
	return !killed;
}

/**
 *  scheduler_add_process - Adds a process to the process queue
 * 
 *  @p - The process to add to the queue, must have at least one thread.
 *  \returns 0 if succeeded, 1 if an error occurred.
 */
int scheduler_add_process(struct process *p)
{
	int rc = 0;
	int placed = 0;
	
	/* switch off scheduler to prevent damage */
	interrupt_ctl(0);
	
	/* check if the process actually exists and has atleast one thread */
	if(!p || !p->threadslen)
		goto out;

	/* find free spot in the process queue */
	int i = 0;
	for(i = 0; i < MAX_PROCESSES; i++) {
		if(!processes[i]) {
			placed = 1;
			break;
		}
	}

	if(! placed)
		goto out;

	/* actually add the process */
	processes[i] = p;

	/* return now */
	interrupt_ctl(1);
	return p->pid;
		
out:
	interrupt_ctl(1);
	return 0;
}

struct process *get_process()
{
	return __p;
}

/**
 * 
 * scheduler_switch - Switches threads
 * 
 * $ARCH must call this when their timer hits.
 * We are in IRQ context, no time constrains! :-)
 */
void scheduler_switch()
{
	SAVE_REGISTERS_STACK;
	__p = 0;
	/* select a process */
	while(!__p) {
		last_process++;
		if (last_process >= MAX_PROCESSES) {
			last_process = 0;
		}
		__p = processes[last_process];
	}
	/* select a thread */
	__t = 0;
	while(!__t) {
		__p->lastthread++;
		if (__p->lastthread >= MAX_THREADS) {
			__p->lastthread = 0;
		}
		__t = __p->threads[__p->lastthread];
	}
	LOAD_REGISTERS_STACK;
	SWITCH_THREAD(__t);
}

/**
 * scheduler_ctl - Control the scheduler 
 * 
 * @enable - possible values:
 * 		1 -> enable the scheduler
 * 		2 -> disable the scheduler
 * 
 * \returns 0 on success
 */
int scheduler_ctl(int enable)
{
	scheduler_status = enable;
	return 0;
}

void task_1()
{
	while(1) {
		tty_write(0, (uint8_t *)"1", 1);
		tty_flush(0);
	}
}

void task_2()
{
	while(1) {
		tty_write(0, (uint8_t *)"2", 1);
		tty_flush(0);
	}
}

/**
 * __start_sched - Start tasking by enabling ARCH timers and jumping
 * 		to the first task
 * 
 * \returns void
 */
void __start_sched()
{
	/* allocate a space for the new kernel stack */
	kernel_stack = (uint32_t) malloc(8192);
	/* move the stack a bit up */
	kernel_stack += 8188;
	kernel_stack &= 0xffffff0;
	interrupt_ctl(1);
	__new__thread = processes[0]->threads[0];
	switch_to_thread();
}

/**
 * scheduler_init - Initialize the scheduling subsystem
 * 
 * Starts tasking.
 * 
 * \returns nothing. starts tasking
 */
void scheduler_init()
{
	tty_write(0, (uint8_t *)"Initializing scheduler\n", 24);
	tty_flush(0);
	
	memset(processes, 0, sizeof(uint32_t) * MAX_PROCESSES);
	
	int rc = scheduler_add_process(create_new_process((uint8_t *)"idletsk", (uint32_t)idle_task));
	if (rc) {
		printk("Idle task failed to create, halted.\n");
		for(;;);
	}
	
	__start_sched();
}
