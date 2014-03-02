#include <mutex.h>
#include <scheduler.h>

static int __all_unlocked = 0;

void unlock_all_mutexes()
{
	printk("Unlocking ALL mutexes! FATAL errors could arise!\n");
	__all_unlocked = 1;
}

void mutex_lock(mutex *m)
{
	while(m->locked && !__all_unlocked) schedule_noirq();
	m->locked = 1;
}

void mutex_unlock(mutex *m)
{
	m->locked = 0;
	schedule_noirq();
}
