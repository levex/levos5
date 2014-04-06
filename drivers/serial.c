#include <hal.h>
#include <display.h>

#define SERIAL_PORT 0x3F8

static int serial_output = 1;

void init_serial() {
	outportb(SERIAL_PORT + 1, 0x00);
	outportb(SERIAL_PORT + 3, 0x80);
	outportb(SERIAL_PORT + 0, 0x03);
	outportb(SERIAL_PORT + 1, 0x00);
	outportb(SERIAL_PORT + 3, 0x03);
	outportb(SERIAL_PORT + 2, 0xC7);
	outportb(SERIAL_PORT + 4, 0x0B);
}

int serial_is_ready()
{
	return inportb(SERIAL_PORT + 5) & 0x20;
}

void serial_write(char a)
{
	if (!serial_output)
		return;
	while(serial_is_ready() == 0) schedule_noirq();

	outportb(SERIAL_PORT, a);
}
