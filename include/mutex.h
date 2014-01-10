#ifndef __MUTEX__H_
#define __MUTEX__H_

#include <stdint.h>

#define INIT_MUTEX(x) x = {.locked=0}

typedef struct __mutex_struct {
	uint8_t locked;
} mutex;

void mutex_unlock(mutex *m);
void mutex_lock(mutex *m);

#endif
