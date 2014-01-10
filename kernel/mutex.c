#include <mutex.h>
#include <scheduler.h>

void mutex_lock(mutex *m)
{
	while(m->locked) schedule_noirq();
	m->locked = 1;
}

void mutex_unlock(mutex *m)
{
	m->locked = 0;
	schedule_noirq();
}
