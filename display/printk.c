#include <display.h>
#include <string.h>
#include <mutex.h>
#include <tty.h>
#include <misc.h>
#include <scheduler.h>

#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wpointer-sign"

extern int DISPLAY_ONLINE;

static mutex printk_lock = {.locked=0};

void __print_ticks(int tty)
{
	tty_write(tty, "[", 1);
	char _str[32] = {0};
	itoa(arch_get_ticks(), 10, _str);
	for(int i = strlen(_str); i < 8; i++)
		tty_write(tty, "0", 1);
	tty_write(tty, _str, strlen(_str));
	tty_write(tty, "] ", 2);
}

void printk(char *fmt, ...)
{
	if(! DISPLAY_ONLINE) return;
	mutex_lock(&printk_lock);
	va_list ap;
	va_start(ap, fmt);
	char *s = 0;
	__print_ticks(0);
	for(int i = 0; i < strlen(fmt); i++)
	{
		if(fmt[i] == '%') {
			switch(fmt[i + 1]) {
				case 's':
					s = va_arg(ap, char *);
					tty_write(0, s, strlen(s));
					i++;
					break;
				case 'd': {
					int c = va_arg(ap, int);
					char str[32] = {0};
					itoa(c, 10, str);
					tty_write(0, str, strlen(str));
					i++;
					break;
				}
				case 'x': {
					int c = va_arg(ap, int);
					char str[32] = {0};
					itoa(c, 16, str);
					tty_write(0, str, strlen(str));
					i++;
					break;
				}
			}
		} else {
			tty_write(0, &fmt[i], 1);
		}
	}
	tty_flush(0);
	va_end(ap);
	mutex_unlock(&printk_lock);
}


void panic(char *buf)
{
    scheduler_ctl(0);
    interrupt_ctl(0);
    mutex_unlock(&printk_lock);
	printk("Kernel panic: ");
    printk(buf);
    for(;;);
}
