#include <display.h>
#include <textmode.h>
#include <mm.h>
#include <mutex.h>

#pragma GCC diagnostic ignored "-Wsign-compare"

#define AS_TEXTMODE_PRIVATE(x) ((struct textmode_private *)(x))

#define TM_COLOR(m) ((AS_TEXTMODE_PRIVATE(m->priv)->bgcol << 4) | (AS_TEXTMODE_PRIVATE(m->priv)->fgcol))

struct textmode_private {
	uint32_t x;
	uint32_t y;
	uint32_t last_x;
	uint32_t last_y;
	uint32_t last_page;
	uint8_t bgcol;
	uint8_t fgcol;
	mutex m;
};

struct textmode_private priv = {
	.x = 0,
	.y = 0,
	.last_page = 0,
	.bgcol = 0x0,
	.fgcol = 0xf,
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
	memset16((char *)(0xb8000 + 25*80*2), TM_COLOR(m) << 8 | ' ', 80*2);
}

uint8_t escape_state = 0;

void textmode_putchar(struct display *m, char c, uint8_t fgcol, uint8_t bgcol)
{
	if(!c)
		return;

	if(AS_TEXTMODE_PRIVATE(m->priv)->x > 80 || c == '\n') {
		AS_TEXTMODE_PRIVATE(m->priv)->y ++;
		AS_TEXTMODE_PRIVATE(m->priv)->x = 0;
	}
	if(AS_TEXTMODE_PRIVATE(m->priv)->y >= 25) {
		textmode_scrollup(m);
		AS_TEXTMODE_PRIVATE(m->priv)->last_page = m->mtty->bufpos;
		AS_TEXTMODE_PRIVATE(m->priv)->x = 0;
		AS_TEXTMODE_PRIVATE(m->priv)->y --;
		AS_TEXTMODE_PRIVATE(m->priv)->last_x = AS_TEXTMODE_PRIVATE(m->priv)->x;
		AS_TEXTMODE_PRIVATE(m->priv)->last_y = AS_TEXTMODE_PRIVATE(m->priv)->y;
	}
	if(c == '\n')
		goto out;
	uint32_t index = 80*2 * AS_TEXTMODE_PRIVATE(m->priv)->y + AS_TEXTMODE_PRIVATE(m->priv)->x*2;
	*(uint16_t *)(0xb8000 + index) = (bgcol << 4 | fgcol) << 8 | c;
	AS_TEXTMODE_PRIVATE(m->priv)->x ++;

out:
	return;
}

void process_ansi_escape(uint8_t *fgcol, uint8_t *bgcol, struct tty *mtty, int *j)
{
	fgcol = fgcol;
	bgcol = bgcol;
	mtty = mtty;
	j = j;
#if 0
	/* \[0;34;11m */
	int start = *j;
	int end = start;
	
	while(mtty->buffer[end] != 'm' && end < mtty->bufpos)
		end ++;
	
	if(end == start)
		return;
	
	/* first find how many ';' do we have */
	int num = 0;
	for(int i = start; i < end; i++)
		if(mtty->buffer[i] == ';')
			num ++;
	
	/* now collect the numbers */
	if (num == 2) {
		/* so we have actually three numbers */
		char *b = &mtty->buffer[end];
		printk("B = %s\n", b);
	}
#endif
}

int textmode_update(struct display *m)
{
	mutex_lock(&AS_TEXTMODE_PRIVATE(m->priv)->m);
	AS_TEXTMODE_PRIVATE(m->priv)->x = AS_TEXTMODE_PRIVATE(m->priv)->last_x;
	AS_TEXTMODE_PRIVATE(m->priv)->y = AS_TEXTMODE_PRIVATE(m->priv)->last_y;
	uint8_t fgcol = AS_TEXTMODE_PRIVATE(m->priv)->fgcol;
	uint8_t bgcol = AS_TEXTMODE_PRIVATE(m->priv)->bgcol;
	for(int i = AS_TEXTMODE_PRIVATE(m->priv)->last_page; i < m->mtty->bufpos; i++)
	{
		if(m->mtty->buffer[i] == '\\') {
			process_ansi_escape(&fgcol, &bgcol, m->mtty, &i);
		}
		if(m->mtty->buffer[i] == '\n') {
			AS_TEXTMODE_PRIVATE(m->priv)->fgcol = fgcol;
			AS_TEXTMODE_PRIVATE(m->priv)->bgcol = bgcol;
		}
		textmode_putchar(m, m->mtty->buffer[i], fgcol, bgcol);
	}
	uint16_t pos = AS_TEXTMODE_PRIVATE(m->priv)->y * 80;
	pos += AS_TEXTMODE_PRIVATE(m->priv)->x;
	outportb(0x3D4, 0x0F);
	outportb(0x3D5, (uint8_t)pos & 0xFF);
	outportb(0x3D4, 0x0E);
	outportb(0x3D5, (uint8_t)(pos >> 8) & 0xFF);
	mutex_unlock(&AS_TEXTMODE_PRIVATE(m->priv)->m);
	return 0;
}

int textmode_setactive(struct display *m)
{
	m = m;
	for(int i = 0; i < 2000; i++)
	{
		*(uint16_t *)(0xB8000 + i*2) = TM_COLOR(m) << 8 | ' ';
	}
	AS_TEXTMODE_PRIVATE(m->priv)->last_x = 0;
	AS_TEXTMODE_PRIVATE(m->priv)->last_y = 0;
	AS_TEXTMODE_PRIVATE(m->priv)->last_page = 0;
	return 0;
}

int textmode_getpos(struct display *m, int *x, int *y) {
	*x = AS_TEXTMODE_PRIVATE(m->priv)->x;
	*y = AS_TEXTMODE_PRIVATE(m->priv)->y;
	return 0;
}

struct display __default_textmode = {
	.update = textmode_update,
	.putchar = textmode_putchar,
	.setactive = textmode_setactive,
	.getpos = textmode_getpos,
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
	memcpy((uint8_t *)p, (uint8_t *)&priv, sizeof(struct textmode_private));
	cp->priv = p;
	/* set the tty of the copied display */
	cp->mtty = mtty;
	/* no need to set the tty's disp field, as we are called so that our
	 * return value will become the tty's disp field.
	 */
	return cp;
}
