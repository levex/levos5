#include <stdint.h>
#include <dbus.h>
#include <mm.h>
#include <mutex.h>
#include <scheduler.h>
#include <syscall.h>

struct dbus_message **messages = 0;
int num_messages = 0;
struct dbus_listener **listeners = 0;
int num_listeners = 0;

mutex INIT_MUTEX(dbus_lock);

int dbus_init()
{
	messages = (struct dbus_message **)malloc(sizeof(uintptr_t) * DBUS_MAX_MSGS);
	listeners = (struct dbus_listener **)malloc(sizeof(uintptr_t) * DBUS_MAX_LSTS);
	printk("D-BUS interface initialized.\n");
	return 0;
}

int dbus_register_listener(struct dbus_listener *lst)
{
	if (!lst)
		return -EINVAL;

	for (int i = 0; i < DBUS_MAX_LSTS; i++)
	{
		if (listeners[i])
			continue;

		listeners[i] = lst;
		return 0;
	}

	return 1;
}

struct dbus_message *__msg = 0;
struct dbus_listener *__lst = 0;
void __dbus_stub()
{
	struct dbus_message *__local_msg = __msg;
	struct dbus_listener *__local_lst = __lst;
	mutex_unlock(&dbus_lock);
	sys_exit(__local_lst->callback(__local_msg));
}

int dbus_send_message(struct dbus_message *msg)
{
	mutex_lock(&dbus_lock);
	for (int i = 0; i < DBUS_MAX_LSTS; i++)
	{
		if (!listeners[i])
			continue;
		if (listeners[i]->msg == msg->msg)
		{
			struct dbus_listener *lst = listeners[i];
			if (lst->type == DBUS_LISTEN_TYPE_ONESHOT)
				listeners[i] = 0;
			
			__msg = msg;
			__lst = lst;
			scheduler_add_process(create_new_process("dbus_handler", __dbus_stub));
		}
	}
	mutex_lock(&dbus_lock);
	mutex_unlock(&dbus_lock);
	return 0;
}
