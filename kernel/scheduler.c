#include <hal.h>
#include <timer.h>
#include <mm.h>
#include <scheduler.h>
#include <string.h>

#pragma GCC diagnostic ignored "-Warray-bounds"

static uint8_t scheduler_status = 0;

static uint8_t last_process = 0;
struct process *processes[MAX_PROCESSES] = {0};
struct thread *mt = 0;

void idle_task()
{
	tty_write(tty_current(), (uint8_t *)"Tasking online.\n", 16);
	tty_flush(tty_current());
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
 * 
 * \returns 0 on success, 1 on failure
 */ 
int create_new_thread(struct process *p, uint32_t s)
{
	int rc = 0;
	
	/* first, switch off scheduler to avoid broken processes */
	scheduler_ctl(0);
	
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
		
	/* set the thread's entry point */
	t->ip = s;
	
	/* allocate space for the thread's stack */
	uint32_t *stack = (uint32_t *)((uint32_t )malloc(THREAD_STACK_SIZE) & 0xfffffff0);
	if(!stack)
		goto err_s;
	stack += 1023;
	uint32_t stacktop = (uint32_t) stack;
		
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
	t->stack = (uint32_t) stack;
		
	/* put the thread into the process */
	p->threads[p->threadslen] = t;
	
	/* increment the number of threads controlled by the process */
	p->threadslen ++;
	
	return 0;
err_s:
	free(t);
err:
	rc = 1;
	scheduler_ctl(1);
	return rc;
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

	/* allocate space for the process */
	struct process *p = malloc(sizeof(struct process));
	if(!p)
		goto err;

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
	
	/* and now create a new thread in the process */
	rc = create_new_thread(p, addr);
	if(rc)
		goto err_3;
		
	return p;

err_3:
	free(name);
err_2:
	free(p);
err:
	return 0;
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
	
	/* switch off scheduler to prevent damage */
	scheduler_ctl(0);
	
	/* check if the process actually exists and has atleast one thread */
	if(!p || !p->threadslen)
		goto err;
		
	/* find free spot in the process queue */
	int i = 0;
	for(i = 0; i < MAX_PROCESSES; i++)
		if(!processes[i])
			break;
	
	/* actually add the process */
	processes[i] = p;
	
	/* return now */
	goto out;
		
err:
	rc = 1;
out:
	scheduler_ctl(1);
	return rc;
}

struct thread *__old__thread = 0;
struct thread *__new__thread = 0;

/* two variables to be used by the $arch */
uint32_t kernel_stack = 0;
uint32_t user_stack = 0;

static struct thread *__t = 0;
static struct process *__p = 0;

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
	scheduler_ctl(1);
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
	
	int rc = scheduler_add_process(create_new_process((uint8_t *)"idletsk", (uint32_t)idle_task));
	if(rc) {
		panic((uint8_t *)"Couldn't create idle task!");
	}
	struct process *testproc = create_new_process((uint8_t *)"test", (uint32_t)task_1);
	create_new_thread(testproc, (uint32_t)task_2);
	rc = scheduler_add_process(testproc);
	if(rc) {
		tty_write(0, (uint8_t *)"Couldn't create test task!", 26);
		tty_flush(0);
	}
	
	__start_sched();
}
