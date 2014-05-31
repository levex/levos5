#include <tty.h>
#include <console.h>

#define KMSG_BUF_SIZE 65536
char *kmsg = 0;

static int kmsg_pos = 0;
static int kmsg_last_flush = 0;

#define MAX_CONSOLES 4
struct console **consoles;
static int num_consoles;

int console_init() {
	int rc;

	kmsg = malloc(KMSG_BUF_SIZE);
	if (!kmsg)
		return -ENOMEM;

	consoles = malloc(sizeof(uint32_t) * MAX_CONSOLES);
	if (!consoles)
		return -ENOMEM;

	num_consoles = 0;
	kmsg_pos = 0;

	printk("kmsg buffer setup. console is now online!\n");

	return 0;
}

int console_add(struct console *c)
{
	if (!c)
		return -EINVAL;

	if (num_consoles >= MAX_CONSOLES)
		return -ENOSPC;

	consoles[num_consoles ++] = c;

	return 0;
}

void console_append(char c)
{
	if (kmsg_pos >= KMSG_BUF_SIZE) {
		kmsg = realloc(kmsg, kmsg_pos + KMSG_BUF_SIZE);
		if (!kmsg)
			panic("kmsg buffer full and no more memory!\n");
	}

	kmsg[kmsg_pos ++] = c;
}

void console_flush()
{
	int i;

	for (i = 0; i < num_consoles; i++) {
		consoles[i]->write(&kmsg[kmsg_last_flush],
		                  kmsg_pos - kmsg_last_flush);
	}

	if (num_consoles)
		kmsg_last_flush = kmsg_pos;
}

void console_write(char *str, int len) {
	while(len --)
		console_append(*str ++);

	console_flush();
}
