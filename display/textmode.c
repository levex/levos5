#include <display.h>
#include <textmode.h>
#include <mm.h>

#pragma GCC diagnostic ignored "-Wsign-compare"

int textmode_update(struct display *m)
{
	/* TODO: handle newlines! */
	for(int i = 0; i < m->mtty->bufpos; i++)
	{
		uint32_t index = 0xB8000 + i*2;
		*(uint16_t *)(index) = 0x1f << 8 | m->mtty->buffer[i];
	}
	return 0;
}

void textmode_putchar(struct display *m, char c)
{
	m = m;
	c = c;
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
	/* set the tty of the copied display */
	cp->mtty = mtty;
	/* no need to set the tty's disp field, as we are called so that our
	 * return value will become the tty's disp field.
	 */
	return cp;
}
