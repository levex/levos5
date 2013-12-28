/* string.h
 * TODO:
 *  much
 */

#ifndef STRING_H
#define STRING_H

#include <stdint.h>

void *memcpy1(void *dest, const void *src, uint32_t count);
void memset1(void *dest, uint8_t value, uint32_t count);
void memsetw1(void *dest, uint16_t value, uint32_t count);


#endif
