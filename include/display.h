#ifndef __DISPLAY_H_
#define __DISPLAY_H_

#include <tty.h>

struct tty;
struct display;

struct display {
	int (*update)(struct display *m);
	void (*putchar)(struct display *m, char c);
	int (*setactive)(struct display *m);
	struct tty *mtty;
};

#endif
