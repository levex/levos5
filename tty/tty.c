#include <stdint.h>
#include <mm.h>
#include <tty.h>
#include <textmode.h>
#include <hal.h>
#include <string.h>
#include <mutex.h>
#include <misc.h>
#include <device.h>
#include <errno.h>

#pragma GCC diagnostic ignored "-Wsign-compare"

struct tty *tty_s;

int ctty = 0;


int tty_generic_write(struct tty *m, uint8_t *buf, uint32_t len)
{
	if(m->bufpos + len > m->buflen)
		return -ENOSPC;
	mutex_lock(&m->m_lock);
	memcpy(m->buffer + m->bufpos, buf, len);
	m->bufpos += len;
	mutex_unlock(&m->m_lock);
	return 0;
}

int tty_generic_flush(struct tty *m)
{
	if(! (m->flags & TTY_FLAG_ACTIVE))
		return -ENOTTY;
	if(m->disp->update)
		return m->disp->update(m->disp);
	return -ENOSYS;
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
		tty_s[ctty].disp = arch_new_default_display(&tty_s[ctty]);
	}
	/* call the tty's event handler of setactive */
	tty_s[ctty].setactive(&tty_s[ctty]);
}

int tty_push_byte(int id, uint8_t c)
{
	struct tty *m = &tty_s[id];
	
	m->inbuf[m->inbuflen] = c;
	m->inbuflen ++;
	return 0;
}

struct tty *tty_get(int id) {
	return &tty_s[id];
}

int tty_generic_read(struct tty *m, uint8_t *buf, uint32_t len)
{
	int copied = 0;
	//mutex_lock(&m->m_lock);
	
	/* if we don't have anything in the input buffer, just return 0 */
	if(m->inbuflen == 0 || !len) goto out;
	
	//printk("Trying to read %d bytes\n", len);
	
	/* try to copy as much as we can */
	for(int i = 0; i < len; i++) {
		/* if the input buffer has this byte... */
		if(m->inbuf[i] && i <= m->inbuflen) {
			/* ... then copy over */
			buf[i] = m->inbuf[i];
			/* increment stat variable */
			copied ++;
		/* ... if no, then break! */
		} else break;
	}
	if(copied == 0) goto out;
	//printk("Copied = %d\n", copied);
	/* since we copied some bytes, substract from buffer length */
	m->inbuflen -= copied;
	
	/* copy down (in the fifo) the remaining bytes */
	memcpy(m->inbuf, m->inbuf + copied, TTY_BUFFER_SIZE - copied);
	

out: //mutex_unlock(&m->m_lock);
	
	return copied;
}

int tty_read(int id, uint8_t *buf, uint32_t len)
{
	struct tty *m = &tty_s[id];
	return m->read(m, buf, len);
}

int tty_current()
{
	return ctty;
}

int tty_generic_dev_write(struct device *dev, uint8_t *buf, uint32_t st, uint32_t len)
{
	/* TODO */
	dev = dev;
	buf = buf;
	len = len;
	st = st;
	return 0;
}

int tty_init(int ttys)
{
	tty_s = malloc(sizeof(struct tty) * ttys);
	if (!tty_s)
		return -ENOMEM;
	memset(tty_s, 0, sizeof( struct tty ));
	for(int i = 0; i < ttys; i++)
	{
		tty_s[i].id = i;
		tty_s[i].flags = 0;
		tty_s[i].write = tty_generic_write;
		tty_s[i].read = tty_generic_read;
		tty_s[i].setactive = tty_generic_setactive;
		tty_s[i].flush = tty_generic_flush;
		tty_s[i].buffer = malloc(TTY_BUFFER_SIZE);
		if(!tty_s[i].buffer)
			return -ENOMEM;
		tty_s[i].buflen = TTY_BUFFER_SIZE;
		tty_s[i].bufpos = 0;
		tty_s[i].inbuf = malloc(TTY_BUFFER_SIZE);
		
		if(!tty_s[i].inbuf)
			return -ENOMEM;
		tty_s[i].inbuflen = 0;
		tty_s[i].disp = arch_new_default_display(&tty_s[i]);
		tty_s[i].inp = arch_new_default_input(&tty_s[i]);
		
		struct device *tdev = malloc(sizeof(struct device));
		if (!tdev)
			return -ENOMEM;
		tdev->id = tty_s[i].id;
		uint8_t *namebuf = malloc(32);
		if (!namebuf)
			return -ENOMEM;
		memcpy(namebuf, (uint8_t*)"tty", 3);
		itoa(tty_s[i].id, 10, (char *) (namebuf + 3));
		tdev->valid = 1;
		tdev->name = (char *)namebuf;
		tdev->write = tty_generic_dev_write;
		
		device_register(tdev);
	}
	return 0;
}

void tty_write(int id, uint8_t *buf, uint32_t len)
{
	tty_s[id].write(&tty_s[id], buf, len);
}

void tty_flush(int id)
{
	tty_s[id].flush(&tty_s[id]);
}
