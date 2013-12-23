#ifndef __TTY_H_
#define __TTY_H_

#include <display.h>
#include <stdint.h>

#define TTY_FLAG_ACTIVE 1 /* tty is the current output stream */
#define TTY_FLAG_ONLINE 2 /* tty is online and processes input */

#define TTY_BUFFER_SIZE 4192

struct display;
struct tty;

struct tty {
	int id;
	uint32_t flags;
	int (*write)(struct tty *m, uint8_t *buf, uint32_t len); /* write to internal tty buffer */
	int (*flush)(struct tty *m); /* flush tty buffer to underlying screen */
	int (*setactive)(struct tty *m);
	struct display *disp;
	uint8_t *buffer;
	uint32_t buflen;
	uint32_t bufpos;
};

extern int tty_init();

extern void tty_write(int id, char *buf, uint32_t len);
extern void tty_flush(int id);

extern void switch_to_tty(int id);

#endif
