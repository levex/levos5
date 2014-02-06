#ifndef __SYSCALL_H_
#define __SYSCALL_H_

#include <stdint.h>

extern void sys_write(int fd, uint8_t *buf, int len);

#endif
