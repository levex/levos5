#include <x86.h>
#include <hal.h>

#define PIC_MASTER_CMD 0x20
#define PIC_MASTER_DATA 0x21
#define PIC_SLAVE_CMD 0xA0
#define PIC_SLAVE_DATA 0xA1

#define PIC_CMD_EOI 0x20

int pic_init()
{
        /* We have to remap the IRQs to be able to process
         * hardware-related interrupt requests and to service
         * exceptions as well.
         */

        /* First step is to save current masks, set by BIOS */
        /* set up cascading mode */
        outportb(PIC_MASTER_CMD, 0x10 + 0x01);
        outportb(PIC_SLAVE_CMD, 0x10 + 0x01);
        /* Setup master's vector offset */
        outportb(PIC_MASTER_DATA, 0x20);
        /* Tell the slave its vector offset */
        outportb(PIC_SLAVE_DATA, 0x28);
        /* Tell the master that he has a slave */
        outportb(PIC_MASTER_DATA, 4);
        outportb(PIC_SLAVE_DATA, 2);
        /* Enabled 8086 mode */
        outportb(PIC_MASTER_DATA, 0x01);
        outportb(PIC_SLAVE_DATA, 0x01);

        outportb(PIC_MASTER_DATA, 0);
        outportb(PIC_SLAVE_DATA, 0);
        return 0;
}

void pic_send_eoi(uint8_t irq)
{
        if(irq >= 8)
                outportb(PIC_SLAVE_CMD, PIC_CMD_EOI);
        outportb(PIC_MASTER_CMD, PIC_CMD_EOI);
}

/* arch must implement! */
int send_eoi(int no)
{
	pic_send_eoi(no);
}
