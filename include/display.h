#ifndef __DISPLAY_H_
#define __DISPLAY_H_

#include <tty.h>
#include <stdarg.h>

struct tty;
struct display;

struct display {
	int (*update)(struct display *m);
	void (*putchar)(struct display *m, char c);
	int (*setactive)(struct display *m);
	int (*getpos)(struct display *m, int *x, int *y);
	void *priv;
	struct tty *mtty;
};

extern void printk(char *fmt, ...);
extern void panic(char *buf);

#endif
