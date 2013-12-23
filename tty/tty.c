#include <stdint.h>
#include <mm.h>
#include <tty.h>
#include <textmode.h>

struct tty *tty_s;

int ctty = 0;


int tty_generic_write(struct tty *m, uint8_t *buf, uint32_t len)
{
	if(m->bufpos + len > m->buflen) return 1;
	memcpy(m->buffer + m->bufpos, buf, len);
	m->bufpos += len;
	return 0;
}

int tty_generic_flush(struct tty *m)
{
	if(! (m->flags & TTY_FLAG_ACTIVE)) return 1;
	if(m->disp->update)
		return m->disp->update(m->disp);
	return 1;
}

int tty_generic_setactive(struct tty *m)
{
	if(m->disp->setactive)
		m->disp->setactive(m->disp);
	m->flush(m);
	return 0;
}

void switch_to_tty(int id)
{
	/* set old tty unactive */
	tty_s[ctty].flags &= ~TTY_FLAG_ACTIVE;
	/* set current tty to the id */
	ctty = id;
	/* set current tty as active */
	tty_s[ctty].flags |= TTY_FLAG_ACTIVE;
	/* check if the current tty has a display */
	if(!tty_s[ctty].disp)
	{
		/* if no, create one */
		tty_s[ctty].disp = textmode_display_new(&tty_s[ctty]);
	}
	/* call the tty's event handler of setactive */
	tty_s[ctty].setactive(&tty_s[ctty]);
}

int tty_init(int ttys)
{
	tty_s = malloc(sizeof(struct tty) * ttys);
	if (!tty_s)
		return 1;
	for(int i = 0; i < ttys; i++)
	{
		tty_s[i].id = i;
		tty_s[i].flags = 0;
		tty_s[i].write = tty_generic_write;
		tty_s[i].setactive = tty_generic_setactive;
		tty_s[i].flush = tty_generic_flush;
		tty_s[i].buffer = malloc(TTY_BUFFER_SIZE);
		if(!tty_s[i].buffer)
			return 1;
		tty_s[i].buflen = TTY_BUFFER_SIZE;
		tty_s[i].bufpos = 0;
		tty_s[i].disp = textmode_display_new(&tty_s[i]);
	}
	return 0;
}


void tty_write(int id, char *buf, uint32_t len)
{
	tty_s[id].write(&tty_s[id], (uint8_t *)buf, len);
}

void tty_flush(int id)
{
	tty_s[id].flush(&tty_s[id]);
}
