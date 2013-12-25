#ifndef __ARM_H_
#define __ARM_H_

#include <tty.h>

#define INT_START ;
#define INT_END ;
        
#define IRQ_START ;

#define IRQ_END ;

extern struct display *pl110_create_new(struct tty *mtty);

#endif
