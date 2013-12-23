/* @author Levente Kurusa <levex@linux.com> */
#include <mm.h>
#include <tty.h>

void main()
{
	int rc;
	
	rc = paging_init();
	if (rc)
		return;

	rc = tty_init(3);
	if (rc)
		return;

	switch_to_tty(0);
	tty_write(0, "HELLO, WORLD", 12);
	tty_flush(0);
	
	switch_to_tty(1);
	tty_write(1, "HELLO, TTY1!", 12);
	tty_flush(1);
	
	switch_to_tty(2);
	tty_write(2, "HELLO, TTY2!", 12);
	tty_flush(2);
	
	switch_to_tty(0);
	
	for(;;);
}
