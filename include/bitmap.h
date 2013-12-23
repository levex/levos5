#ifndef __BITMAP_H_
#define __BITMAP_H_

#include <stdint.h>

extern void set_bitmap(uint8_t *b, int i);
extern void unset_bitmap(uint8_t *b, int i);
extern int get_bitmap(uint8_t *b, int i);
extern int bitmap_find_zero(uint8_t *b, uint32_t nr);

#endif
