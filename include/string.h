#ifndef __STRING_H_
#define __STRING_H_

#include <stddef.h>
#include <stdint.h>

extern size_t strlen(const uint8_t *str);
extern size_t strcmp(char *str1, char *str2);
extern size_t str_backspace(char *str, char c);
extern size_t strsplit(char *str, char delim);

#endif
