#include <stdint.h>
#include <mm.h>
#include <display.h>


static unsigned int *ebp = 0;
static unsigned int i = 0;
static unsigned int eip = 0;
static unsigned int *args = 0;
void __stack_trace(unsigned int frames)
{
	ebp = &frames - 2;
	printk("Stack trace:\n");
	for (i = 0; i < frames; i++)
	{
		eip = ebp[0];
		if (!eip)
			break;
		printk("0x%x\n", eip);
		ebp = (unsigned int *)&ebp[0] - 2;
	}
}

void stack_trace()
{
//	__stack_trace(16);
}
