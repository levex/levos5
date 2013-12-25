#ifndef __KEYBOARD_H_
#define __KEYBOARD_H_

extern int keyboard_init();
extern struct input *keyboard_create_new(struct tty *mtty);

#endif
