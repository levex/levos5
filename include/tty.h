#ifndef __TTY_H_
#define __TTY_H_

#include <display.h>
#include <stdint.h>
#include <mutex.h>

#define TTY_FLAG_ACTIVE 1 /* tty is the current output stream */
#define TTY_FLAG_ONLINE 2 /* tty is online and processes input */

#define TTY_BUFFER_SIZE 4192

struct display;
struct input;
struct tty;

struct tty {
	int id;
	uint32_t flags;
	int (*write)(struct tty *m, uint8_t *buf, uint32_t len); /* write to internal tty buffer */
	int (*flush)(struct tty *m); /* flush tty buffer to underlying screen */
	int (*read) (struct tty *m, uint8_t *buf, uint32_t len); /* read from connected input source */
	int (*setactive)(struct tty *m);
	struct display *disp;
	struct input   *inp;
	mutex m_lock;
	uint8_t *buffer;
	uint32_t buflen;
	uint32_t bufpos;
	uint8_t *inbuf;
	uint32_t inbuflen;
};

extern int tty_init();

extern void tty_write(int id, uint8_t *buf, uint32_t len);
extern void tty_flush(int id);

extern int tty_read(int id, uint8_t *buf, uint32_t len);
extern int tty_push_byte(int id, uint8_t c);

extern void switch_to_tty(int id);
extern struct tty *tty_get(int id);
extern int tty_current();

#endif
