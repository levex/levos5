#include <bitmap.h>
#include <stdint.h>

#pragma GCC diagnostic ignored "-Wsign-compare"

void set_bitmap(uint8_t *b, int i) {
    b[i / 8] |= 1 << (i & 7);
}

void unset_bitmap(uint8_t *b, int i) {
    b[i / 8] &= ~(1 << (i & 7));
}

int get_bitmap(uint8_t *b, int i) {
    return b[i / 8] & (i & 7) ? 1 : 0;
}

int bitmap_find_zero(uint8_t *b, uint32_t nr)
{
	int streak = 0;
	int start = 0;
	
	if (!nr) return 0;
	
	while(streak != nr)
	{
		if (!get_bitmap(b, start + streak)) {
			streak ++;
		} else {
			start += streak + 1;
			streak = 0;
		}
	}
	for(int i = start; i < start + streak; i++)
		set_bitmap(b, i);
	
	//printk("request of bitmap: %d -> %d\n", start, start+nr);
	
	return start;
}
