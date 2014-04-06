#ifndef __DBUS__H_
#define __DBUS__H_

#define DBUS_LISTEN_TYPE_ONESHOT 1
#define DBUS_LISTEN_TYPE_CONTINUOUS 2

#define DBUS_EVENT_PROCESS_KILLED 1
#define DBUS_EVENT_INIT_KILLED 2 

#define DBUS_MAX_MSGS 256
#define DBUS_MAX_LSTS 256

struct dbus_message {
	int msg; /* message */
	int id; /* unique identifier */
	uint8_t *buf; /* buffer to data */
	int len; /* length of buffer */
};

struct dbus_listener {
	int msg; /* the message it listens to */
	int type; /* see DBUS_LISTEN_TYPE_* */
	int (*callback)(struct dbus_message *dmg); /* callback */
};

extern int dbus_init();

extern int dbus_register_listener(struct dbus_listener *lst);

extern int dbus_send_message(struct dbus_message *msg);

#endif
