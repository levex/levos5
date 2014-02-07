#include <hal.h>
#include <tty.h>
#include <scheduler.h>
#include <syscall.h>
#include <vfs.h>

/* eax=0x1 ebx=code */
void sys_exit(int code)
{
	struct process *p = get_process();
	printk("process %d exiting with errcode: %d\n", p->pid, code);
	scheduler_kill_self();
}
extern void __sys__fork_cont();
int __imp__fork_cont()
{
	/* we are the child */
	//asm volatile("add $0x14, %esp; pop %ebx; pop %esi");
	return sys_getpid();
}

struct process *parent = 0;
struct process *child = 0;
uint32_t *pd = 0;
uint8_t *buf = 0;
uint32_t *stack = 0;
struct thread *t = 0;
uint32_t stackdiff = 0;
uint32_t esp = 0;
/* eax=0x2 */
int sys_fork()
{
	/* save the current process as the parent process */
	parent = get_process();
	
	/* prepare a stub for the child */
	child = create_new_process_nothread("child");
	/* allocate physical space for the child
	 * we should alloc the same space as the parent */
	child->palloc = phymem_alloc(parent->exec_len);
	
	/* get length of the palloc */
	child->palloc_len = parent->exec_len;
	child->exec_len = parent->exec_len;
	
	/* allocate new page directory for child */
	pd = create_new_page_directory(0x400000, child->palloc);
	child->paged = pd;
	
	/* copy over the data from the parent to the child
	 * first we create a temporary buffer */
	interrupt_ctl(0);
	buf = malloc(parent->palloc_len);
	memset(buf, 0, parent->palloc_len);
	
	/* then we copy to the buf */
	memcpy(buf, 0x400000, parent->palloc_len);
	
	/* then we switch pagedirectory */
	asm volatile("mov %%eax, %%cr3"::"a"(child->paged));
	
	/* then copy over from the buffer to the child */
	memcpy(0x400000, buf, child->palloc_len);
	
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
		memcpy(stack, parent->threads[0]->stackbot, THREAD_STACK_SIZE);
		asm volatile("mov %%esp, %0":"=m"(esp));
		stack = (uint32_t *)((uint32_t)stack + esp - (uint32_t)parent->threads[0]->stackbot);
		replace_page(child->paged, parent->threads[0]->stack, (uint32_t)((uint32_t)rstack | 3));
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

/* eax=0x3 ebx=fd ecx=buf edx=count */
int sys_read(int fd, uint8_t *buf, int len)
{
	struct process *p = get_process();
	struct tty *mt = tty_get(p->tty_id);
	if (fd == 0) {
		int totalread = 0;
		int read = 0;
		while(totalread < len) {
			read = tty_read(p->tty_id, buf + totalread, 1);
			if(read) {
				totalread ++;
				/* we read a byte, check if it is \n */
				if(buf[totalread - 1] == '\n') {
					/* if so, return! */
					tty_write(p->tty_id, (uint8_t *)(buf + totalread - 1), 1);
					tty_flush(p->tty_id);
					return totalread;
				}
				if(buf[totalread - 1] == 0x08) {
					buf[totalread - 0] = 0;
					buf[totalread - 1] = 0;

					mt->buffer[mt->bufpos - 0] = 0;
					mt->buffer[mt->bufpos - 1] = ' ';
					tty_flush(p->tty_id);
					mt->bufpos -= 1;
					mt->buflen -= 1;
					tty_flush(p->tty_id);

					
					totalread -= 2;
					continue;
				}
				tty_write(p->tty_id, (uint8_t *)(buf + totalread - 1), 1);
				tty_flush(p->tty_id);
			}
			schedule_noirq();
		}
		return totalread;
	} else {
		struct file *fl = get_process()->filehandles[fd - 3];
		return vfs_read(fl, buf, len);
	}
	return 0;
}

/* eax=0x4 ebx=fd ecx=buf edx=size */
void sys_write(int fd, uint8_t *buf, int len)
{
	struct process *p = get_process();
	if (fd == 1) { /* stdout */
		tty_write(p->tty_id, buf, len);
		tty_flush(p->tty_id);
	}
}

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR   2
/* eax=0x5 ebx=filename ecx=flags */
int sys_open(char *fn, uint32_t flags)
{
	if(flags != O_RDONLY)
		return -1;
	
	struct process *p = get_process();
	if(p->open_handles >= MAX_FILEHANDLES)
		return -1;
	
	struct file *fl = vfs_open(fn);
	if(!fl)
		return -1;
	
	p->filehandles[p->open_handles] = fl;
	p->open_handles ++;
	return p->open_handles + 2;
}

static uint8_t *ptr = 0;
static uint32_t esize = 0;
void __elf_stub() {
	elf_start(ptr, esize, 0);
	sys_exit(1337);
}

/* eax=0xB ebx=path ecx=argv edx=envp */
int sys_execve(char *path, char *const argv[], char *envp[])
{
	struct stat st;
	struct file *fl = vfs_open(path);
	if(!fl) return 0;
	/* get size of exec */
	sys_stat(fl->fullpath, &st);
	uint32_t size = st.st_size;
	if(!size) return 0;
	//printk("creating a buffer of %d bytes\n", size);
	uint8_t *buf = malloc(size);
	ptr = buf;
	esize = size;
	/* create the argv buffer */
	char **argv_k = 0;
	if(argv) {
		int argc = 0;
		while(argv[argc]) argc ++;
		//printk("%s: argc=%d\n", __func__, argc);
		argv_k = malloc((argc + 1) * sizeof(char *));
		for(int i = 0; i < argc; i++)
		{
			char *str = malloc(strlen(argv[i]) + 1);
			memcpy(str, argv[i], strlen(argv[i]) + 1);
			argv_k[i] = str;
		}
		argv_k[argc] = 0;
	}
	/* read the file in */
	vfs_read_full(fl, buf);
	/* start a new process from the stub */
	elf_start(buf, esize, argv_k, envp, 0);
}

/* eax=0x12 ebx=path ecx=st */
void sys_stat(char *path, struct stat* st)
{
	memset(st, 0, sizeof(struct stat));
	vfs_stat(vfs_open(path), st);
}

/* eax=0x14 */
int sys_getpid()
{
	return get_process()->pid;
}
/* eax=0xed ebx=fn */
int sys_opendir(char *fn)
{
	if(!fn)
		return -1;

	if(fn[strlen(fn)] != '/') {
		char *m = malloc(strlen(fn) + 1);
		memcpy(m, fn, strlen(fn) + 1);
		int s = strlen(fn);
		m[s] = '/';
		m[s + 1] = 0;
		fn = m;
	}
	struct file *fl = vfs_open(fn);
	if(! vfs_isdir(fl)) {
		free(fn);
		free(fl);
		return -1;
	}
	return sys_open(fn, O_RDONLY);
}
/* eax=0xee ebx=dd */
struct dirent *sys_readdir(int dd)
{
	struct file *fl = get_process()->filehandles[dd - 3];

	return vfs_readdir(fl);
}

/* eax=0xef */
void sys_waitpid(int pid)
{
	while(is_pid_running(pid)) schedule_noirq();
	return;
}

struct utsname {
	char sysname[64];
	char nodename[64];
	char release[64];
	char version[64];
	char machine[64];
};

void sys_uname(struct utsname *name)
{
	char *sysname = "LevOS";
	char *nodename = "levos-pc";
	char *release = "rc0";
	char *version = "5.1";
	char *machine = "LevOS/NoGNU";
	
	memcpy(name->sysname, sysname, strlen(sysname));
	memcpy(name->nodename, nodename, strlen(nodename));
	memcpy(name->release, release, strlen(release));
	memcpy(name->version, version, strlen(version));
	memcpy(name->machine, machine, strlen(machine));
	
}
