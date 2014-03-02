#include <hal.h>
#include <tty.h>
#include <scheduler.h>
#include <syscall.h>
#include <string.h>
#include <vfs.h>
#include <mm.h>
#include <elf.h>

#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wpointer-sign"

/* eax=0x1 ebx=code */
void sys_exit(int code)
{
	struct process *p = get_process();
	printk("process %d exiting with errcode: %d\n", p->pid, code);
	scheduler_kill_self();
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
	} else {
		struct file *fl = get_process()->filehandles[fd - 3];
		vfs_write(fl, buf, len);
		return;
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
	return 0;
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
	
	if (strcmp(".", fn) == 0)
		return 0xFF;

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
	if (dd == 0xFF)
		return vfs_readdir(get_process()->workdir);
	struct file *fl = get_process()->filehandles[dd - 3];

	return vfs_readdir(fl);
}

/* eax=0xef */
void sys_waitpid(int pid)
{
	while(is_pid_running(pid)) schedule_noirq();
	return;
}

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
