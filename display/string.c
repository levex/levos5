#include <string.h>


/**
 * Copies a piece of memory from one place to another
 * @param void *dest pointer to destination
 * @param void *src pointer to source
 * @param uint32_t count number of bytes to copy
 * @return return pointer to destination
 */
void *memcpy1(void *dest, const void *src, uint32_t count) {
	const uint8_t *pSource = (const uint8_t *) src;
	uint8_t *pDest = (uint8_t *) dest;
	
	for (; count; --count) *pDest++ = *pSource++;
	return dest;
}

/**
 * Fills a block of emmory with the specified value
 * @param void *dest pointer to memory to fill
 * @param uint8_t value Character/value to fill with
 * @param uint32_t count numebr of bytes to write
 */
void memset1(void *dest, uint8_t value, uint32_t count) {
	uint8_t *pDest = (uint8_t *) dest;
	for (; count; --count) *pDest++ = value;
}

/**
 * Fills a block of memory with the specified value
 * @param void *dest pointer to memory to fill
 * @param uint16_t value Character/value to fill with
 * @param uint32_t count number of words to write
 */
void memsetw1(void *dest, uint16_t value, uint32_t count) {
	uint16_t *pDest = (uint16_t *) dest;
	for (; count; --count) *pDest++ = value;
}
