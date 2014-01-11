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
		
	malloc(1);
		
	rc = tty_init(10);
	if (rc)
		return;
		
	rc = arch_late_init();
	if (rc)
		return;
		
	rc = keyboard_init();
	if (rc) {
		tty_write(2, (uint8_t *) "Keyboard failure!", 18);
		tty_flush(2);
		switch_to_tty(2);
	}
	
	switch_to_tty(0);
	
	scheduler_init();
}

void late_init()
{	
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
