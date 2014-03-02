#include <stdint.h>
#include <display.h>
#include <mm.h>
#include <font.h>

static struct pl110_mmio *pm = 0;
static uint16_t volatile *fb = 0;

#define PL110_IOBASE		0xc0000000

struct pl110_mmio
{
	uint32_t		volatile tim0;		//0
	uint32_t		volatile tim1;		//4
	uint32_t		volatile tim2;		//8
	uint32_t		volatile d;		//c
	uint32_t		volatile upbase;	//10
	uint32_t		volatile f;		//14
	uint32_t		volatile g;		//18
	uint32_t		volatile control;	//1c
};

void pl110_put_pixel(uint32_t x, uint32_t y, uint32_t color)
{
	uint32_t index = x + y * 640;
	if(color)
		fb[index] = 0x10 << (5 + 1) | 0xf << 5;
}

static int _x = 0;
static int _y = 0;

void pl110_putc(char c)
{
	if(c == '\n') {
		_x = 0;
		_y += 8;
		return;	
	}
	int cx, cy;
	int mask[8] = {1,2,4,8,16,32,64,128};
	uint8_t *glyph = g_8x8_font + (int)c * 8;

	for(cy = 0; cy < 8; cy ++)
		for(cx = 0; cx < 8; cx ++)
			pl110_put_pixel(_x+(8-cx), _y+cy, glyph[cy]&mask[cx]?1:0);

	_x += 8;
	if(_x > 640) {
		_x = 0;
		_y += 8;	
	}
}

void pl110_clear()
{
	/*for (int x = 0; x < (640 * 480) - 10; ++x)
		fb[x] = 0x1f << (5 + 6) | 0xf << 5;*/
}

int pl110_setactive(struct display *m)
{
	m=m;
		
	pm = (struct pl110_mmio *)PL110_IOBASE;
	pm->tim0 = 0x3f1f3f9c;
	pm->tim1 = 0x080b61df;
	pm->upbase = 0x200000;
	pm->control = 0x1829;
	fb = (uint16_t*)0x200000;		
	
	for (int x = 0; x < (640 * 480) - 10; ++x)
		fb[x] = 0x1f << (5 + 6) | 0xf << 5;
	
	_x = 0;
	_y = 0;
	
	return 0;
}

int pl110_update(struct display *m)
{
	pl110_setactive(m);
	for(unsigned int i = 0; i < m->mtty->bufpos; i++)
	{
		uint8_t c = m->mtty->buffer[i];
		pl110_putc(c);
	}
	return 0;
}

struct display pl110_display = {
	.setactive = pl110_setactive,
	.update = pl110_update,
	.mtty = 0,
};

struct display *pl110_create_new(struct tty *mtty)
{
	struct display *ret = (struct display *)malloc(sizeof(struct display));
	memcpy((uint8_t *)ret, (uint8_t *)&pl110_display, sizeof(struct display));
	ret->mtty = mtty;
	return ret;
}
