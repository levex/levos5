/* @author Levente Kurusa <levex@linux.com> */
#include <mm.h>
#include <tty.h>
#include <keyboard.h>
#include <hal.h>

void main()
{
	int rc;
	
	rc = arch_early_init();
	if (rc)
		return;
	
	rc = paging_init();
	if (rc)
		return;
		
	rc = tty_init(10);
	if (rc)
		return;
		
	rc = arch_late_init();
	if (rc)
		return;
		
	rc = keyboard_init();
	if (rc) {
		tty_write(2, "Keyboard failure!", 18);
		tty_flush(2);
		switch_to_tty(2);
	}
	
	
	tty_write(1, "tty1", 5);
	tty_flush(1);
	

	tty_write(0, "SYSTEM ONLINE", 13);
	tty_flush(0);
	
	tty_write(2, "second tty", 11);
	tty_flush(2);
	
	switch_to_tty(0);
	interrupt_ctl(1);
	
	uint8_t *buf = malloc(64);
	memset(buf, 0, 64);
	while(1)
	{
		memset(buf, 0, 64);
		int read = tty_read(tty_current(), buf, 64);
		if(!read) continue;
		if( buf[0] >= '0' && buf[0] <= '9' )
		{
			switch_to_tty(buf[0] - '0');
			continue;
		}
		tty_write(tty_current(), buf, read);
		tty_flush(tty_current());
	}
	
	for(;;);
}
