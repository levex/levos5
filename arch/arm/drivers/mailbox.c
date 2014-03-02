#include <stdint.h>
#include <uart.h>
#include <arm.h>
#include "image_data.h"
#include <font.h>

uint32_t mailbox_write(uint32_t fbinfo_addr, uint32_t chan)
{
	uint32_t mailbox = 0;
	
	mailbox = 0x2000B880;
	while(1)
		if((__arm__get32(mailbox+0x18)&0x80000000)==0) break;
	
	__arm__put32(mailbox+0x20, fbinfo_addr+chan);
	return 0;
}

uint32_t mailbox_read(uint32_t chan)
{
	uint32_t ra = 0;
	uint32_t mailbox = 0x2000B880;
	
	while(1) {
		while(1) {
			ra = __arm__get32(mailbox+0x18);
			if((ra&0x40000000)==0) break;
		}
		ra = __arm__get32(mailbox+0x00);
		if((ra&0x0f)==chan) break;
	}
	return ra;
}

int mailbox_init()
{
	printk("%s\n", __func__);
	uint32_t ra, rb;
	uint32_t rx, ry;
	
	__arm__put32(0x40040000, 640); /* #0 Physical Width */
	__arm__put32(0x40040004, 480); /* #4 Physical Height */
	__arm__put32(0x40040008, 640); /* #8 Virtual Width */
	__arm__put32(0x4004000C, 480); /* #12 Virtual Height */
	__arm__put32(0x40040010, 0);   /* #16 GPU - Pitch */
	__arm__put32(0x40040014, 32);  /* #20 Bit Depth */
	__arm__put32(0x40040018, 0);   /* #24 X */
	__arm__put32(0x4004001C, 0);   /* #28 Y */
	__arm__put32(0x40040020, 0);   /* #32 GPU - Pointer */
	__arm__put32(0x40040024, 0);   /* #36 GPU - Size */
	mailbox_write(0x40040000, 1);
	
	rb = __arm__get32(0x40040020);
	for(ra = 0; ra < 10000; ra++) {
		__arm__put32(rb, ~((ra&0xff)<<0));
		rb += 4;
	}
	for(ra = 0; ra < 10000; ra++) {
		__arm__put32(rb, ~((ra&0xff)<<8));
		rb += 4;
	}
	for(ra = 0; ra < 10000; ra++) {
		__arm__put32(rb, ~((ra&0xff)<<16));
		rb += 4;
	}
	for(ra = 0; ra < 10000; ra++) {
		__arm__put32(rb, ~((ra&0xff)<<24));
		rb += 4;
	}
	rb = __arm__get32(0x40040020);
	ra = 0;
	/*for(ry = 0; ry < 480; ry++)
	{
		for(rx = 0; rx < 480; rx++)
		{
			__arm__put32(rb, image_data[ra++]);
			rb += 4;
		}
		for(;rx<640;rx++)
		{
			__arm__put32(rb, 0);
			rb += 4;
		}
	}*/
	/*while(1) {
		__arm__put32(0x2020001C, 1<<16);
		for(ra = 0; ra < 0x100000; ra++) dummy(ra);
		__arm__put32(0x20200028, 1<<16);
		for(ra=0;ra<0x100000;ra++) dummy(ra);
	}*/
}

static uint32_t mailbox_lastpos = 0;
static uint32_t mailbox_x = 0;
static uint32_t mailbox_y = 0;

#define MBPUTPIX(b, x, y, k) __arm__put32(b + y*640*4 + x*4, k?0xFFD18867:0)

void mailbox_putc(char c)
{
	//0xFFD18867
	if(c == '\n') {
		mailbox_x = 0;
		mailbox_y += 8;
		return;	
	}
	int cx, cy;
	int mask[8] = {1,2,4,8,16,32,64,128};
	uint8_t *glyph = g_8x8_font + (int)c * 8;
	uint32_t base = __arm__get32(0x40040020);

	for(cy = 0; cy < 8; cy ++)
		for(cx = 0; cx < 8; cx ++)
			MBPUTPIX(base, mailbox_x+(8-cx), mailbox_y+cy, glyph[cy]&mask[cx]?1:0);

	mailbox_x += 8;
	if(mailbox_x > 640) {
		mailbox_x = 0;
		mailbox_y += 8;	
	}
}

void mailbox_setactive()
{
	int ry, rx, rb;
	rb = __arm__get32(0x40040020);
	
	for(ry = 0; ry < 480; ry++)
	{
		for(rx = 0; rx < 480; rx++)
		{
			__arm__put32(rb, 0);
			rb += 4;
		}
		for(;rx<640;rx++)
		{
			__arm__put32(rb, 0);
			rb += 4;
		}
	}
}
void mailbox_update(struct display *m)
{
	for(int i = mailbox_lastpos; i < m->mtty->bufpos; i++)
	{
		mailbox_putc(m->mtty->buffer[i]);
	}
	mailbox_lastpos = m->mtty->bufpos;
}

struct display __default_mailbox = {
	.update = mailbox_update,
	.setactive = mailbox_setactive,
	.priv = 0,
	.mtty = 0,
};

struct display *mailbox_display_new(struct tty *mtty)
{
	struct display *cp = malloc(sizeof(struct display));
	memcpy((uint8_t *)cp, (uint8_t *)&__default_mailbox, sizeof(struct display));
	cp->mtty = mtty;
	return cp;
}
