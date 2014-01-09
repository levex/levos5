#include <string.h>
#include <stdint.h>
#include <stddef.h>

size_t strlen(const uint8_t *str)
{
        size_t i = 0;
        while(str[i] != 0) i++;
        return i;
}
