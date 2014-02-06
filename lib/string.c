#include <string.h>
#include <stdint.h>
#include <stddef.h>

size_t strlen(const uint8_t *str)
{
        size_t i = 0;
        while(str[i] != 0) i++;
        return i;
}

size_t strcmp(char *str1, char *str2)
{
	size_t res=0;
	while (!(res = *(unsigned char*)str1 - *(unsigned char*)str2) && *str2)
		++str1, ++str2;

	return res;
}

size_t str_backspace(char *str, char c)
{
	size_t i = strlen((uint8_t *)str);
	i--;
	while(i)
	{
		i--;
		if(str[i] == c)
		{
			str[i+1] = 0;
			return 1;
		}
	}
	return 0;
}

size_t strsplit(char *str, char delim)
{
        size_t n = 0;
        uint32_t i = 0;
        while(str[i])
        {
                if(str[i] == delim)
                {
                        str[i] = 0;
                        n++;
                }
                i++;
        }
        n++;
        return n;
}
