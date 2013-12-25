#ifndef __INPUT_H_
#define __INPUT_H_

#include <stdint.h>
#include <tty.h>

struct tty;

struct input {
	int id;
	/* returns the number of bytes read */
	int (*read)(struct input *m, uint8_t *buf, uint32_t len);
	/* tty we are connected to */
	struct tty *mtty;
};

extern int input_register(struct input *m);

#endif
