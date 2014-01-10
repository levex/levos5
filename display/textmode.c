#include <display.h>
#include <textmode.h>
#include <mm.h>
#include <mutex.h>

#pragma GCC diagnostic ignored "-Wsign-compare"

#define AS_TEXTMODE_PRIVATE(x) ((struct textmode_private *)(x))

struct textmode_private {
	uint32_t x;
	uint32_t y;
	uint32_t last_page;
	mutex m;
};

struct textmode_private priv = {
	.x = 0,
	.y = 0,
	.last_page = 0,
	INIT_MUTEX(.m),
};

void textmode_scrollup(struct display *m)
{
	m = m;
	for(int y = 0; y < 25; y++)
	{
		memcpy((uint8_t *)(0xb8000 + y*80*2) ,
				(uint8_t *)(0xb8000 + (y+1)*80*2),
				80*2);
	}
	memset16((char *)(0xb8000 + 25*80*2), 0x1f << 8 | ' ', 80*2);
}

void textmode_putchar(struct display *m, char c)
{
	if(!c)
		return;

	if(AS_TEXTMODE_PRIVATE(m->priv)->x > 80 || c == '\n') {
		AS_TEXTMODE_PRIVATE(m->priv)->y ++;
		AS_TEXTMODE_PRIVATE(m->priv)->x = 0;
	}
	if(AS_TEXTMODE_PRIVATE(m->priv)->y > 25) {
		textmode_scrollup(m);
		AS_TEXTMODE_PRIVATE(m->priv)->last_page += 80;
		AS_TEXTMODE_PRIVATE(m->priv)->x = 0;
		AS_TEXTMODE_PRIVATE(m->priv)->y --;
	}
	if(c == '\n')
		goto out;
	uint32_t index = 80*2 * AS_TEXTMODE_PRIVATE(m->priv)->y + AS_TEXTMODE_PRIVATE(m->priv)->x*2;
	*(uint16_t *)(0xb8000 + index) = 0x1f << 8 | c;
	AS_TEXTMODE_PRIVATE(m->priv)->x ++;

out:
	return;
}

int textmode_update(struct display *m)
{
	mutex_lock(&AS_TEXTMODE_PRIVATE(m->priv)->m);
	AS_TEXTMODE_PRIVATE(m->priv)->x = 0;
	AS_TEXTMODE_PRIVATE(m->priv)->y = 0;
	for(int i = AS_TEXTMODE_PRIVATE(m->priv)->last_page; i < m->mtty->bufpos; i++)
	{
		textmode_putchar(m, m->mtty->buffer[i]);
	}
	mutex_unlock(&AS_TEXTMODE_PRIVATE(m->priv)->m);
	return 0;
}

int textmode_setactive(struct display *m)
{
	m = m;
	for(int i = 0; i < 2000; i++)
	{
		*(uint16_t *)(0xB8000 + i*2) = 0x1f << 8 | ' ';
	}
	return 0;
}

struct display __default_textmode = {
	.update = textmode_update,
	.putchar = textmode_putchar,
	.setactive = textmode_setactive,
	.priv = 0,
	.mtty = 0,
};

struct display *textmode_display_new(struct tty *mtty)
{
	/* allocate space for a new display */
	struct display *cp = malloc(sizeof(struct display));
	/* copy to the new allocation the default settings of textmode */
	memcpy((uint8_t *)cp, (uint8_t *)&__default_textmode, sizeof(struct display));
	/* allocate space for a new priv data */
	struct textmode_private *p = malloc(sizeof(struct textmode_private));
	cp->priv = p;
	/* set the tty of the copied display */
	cp->mtty = mtty;
	/* no need to set the tty's disp field, as we are called so that our
	 * return value will become the tty's disp field.
	 */
	return cp;
}
