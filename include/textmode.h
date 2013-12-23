#ifndef __TEXTMODE_H_
#define __TEXTMODE_H_

#include <display.h>
#include <tty.h>


extern struct display *textmode_display_new(struct tty *mtty);

#endif
