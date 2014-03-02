#include <mm.h>
#include <string.h>
#include <scheduler.h>
#include <hal.h>

#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wpointer-sign"

struct process *parent = 0;
struct process *child = 0;
uint32_t *pd = 0;
uint8_t *buf = 0;
uint32_t *stack = 0;
struct thread *t = 0;
uint32_t stackdiff = 0;
uint32_t esp = 0;

extern void replace_page(uint32_t **pd, uint32_t vaddr, uint32_t page);
extern uint32_t *create_new_page_directory(uint32_t vaddr, uint32_t paddr);

/* eax=0x2 */
int sys_fork()
{
	/* save the current process as the parent process */
	parent = get_process();
	
	/* prepare a stub for the child */
	child = create_new_process_nothread("child");
	/* allocate physical space for the child
	 * we should alloc the same space as the parent */
	child->palloc = (uint32_t)phymem_alloc(parent->exec_len);
	
	/* get length of the palloc */
	child->palloc_len = parent->exec_len;
	child->exec_len = parent->exec_len;

	/* allocate new page directory for child */
	pd = create_new_page_directory(0x400000, child->palloc);
	child->paged = (uint32_t)pd;
	
	/* copy over the data from the parent to the child
	 * first we create a temporary buffer */
	interrupt_ctl(0);
	buf = malloc(parent->palloc_len);
	memset(buf, 0, parent->palloc_len);
	
	/* then we copy to the buf */
	memcpy(buf, (uint8_t *)0x400000, parent->palloc_len);
	
	/* then we switch pagedirectory */
	asm volatile("mov %%eax, %%cr3"::"a"(child->paged));
	
	/* then copy over from the buffer to the child */
	memcpy((uint8_t *)0x400000, buf, child->palloc_len);
	
	/* now switch back the paged directory */
	asm volatile("mov %%eax, %%cr3"::"a"(parent->paged));
	interrupt_ctl(1);
	free(buf);
	
	t = malloc(sizeof(struct thread));
	memset(t, 0, sizeof(struct thread));
	t->paged = &child->paged;
	
	uint32_t eip = read_eip();
	//printk("EIP = 0x%x (%d)\n", eip, get_process()->pid);
	
	if(eip) {
		stack = malloc(THREAD_STACK_SIZE);
		uint32_t rstack = (uint32_t)stack;
		memcpy((uint8_t *)stack, (uint8_t *)parent->threads[0]->stackbot, THREAD_STACK_SIZE);
		asm volatile("mov %%esp, %0":"=m"(esp));
		stack = (uint32_t *)((uint32_t)stack + esp - (uint32_t)parent->threads[0]->stackbot);
		replace_page((uint32_t **)child->paged, parent->threads[0]->stack, (uint32_t)((uint32_t)rstack | 3));
		t->rstack = rstack;
		/* now create the process stub */
		*--stack = 0x00000202; // eflags
		*--stack = 0x8; // cs
		*--stack = eip; // eip
		*--stack = 0; // eax
		*--stack = 0; // ebx
		*--stack = 0; // ecx;
		*--stack = 0; //edx
		*--stack = 0; //esi
		*--stack = 0; //edi
		*--stack = t->stacktop; //ebp
		*--stack = 0x10; // ds
		*--stack = 0x10; // fs
		*--stack = 0x10; // es
		*--stack = 0x10; // gs
		t->ip = eip;
		t->stack = (uint32_t)stack;
		child->threads[0] = t;
		child->threadslen ++;
		
		scheduler_add_process(child);
		return child->pid;
	} else {
		return 0;
	}
}
