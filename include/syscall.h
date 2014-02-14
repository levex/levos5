#ifndef __SYSCALL_H_
#define __SYSCALL_H_

#include <stdint.h>

struct utsname {
	char sysname[64];
	char nodename[64];
	char release[64];
	char version[64];
	char machine[64];
};

extern void sys_write(int fd, uint8_t *buf, int len);
extern void sys_exit(int code);
extern int sys_fork();
extern int sys_read(int fd, uint8_t *buf, int len);
extern int sys_open(char *fn, uint32_t flags);
extern int sys_execve(char *path, char *const argv[], char *envp[]);
extern void sys_stat(char *path, struct stat* st);
extern int sys_getpid();
extern int sys_opendir(char *fn);
extern struct dirent *sys_readdir(int dd);
extern void sys_waitpid(int pid);
extern void sys_uname(struct utsname *name);

#endif
