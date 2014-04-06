#include <stdio.h>
#include <string.h>

void main(void)
{
    char *argv[] = {"hello", "world", 0};
    int argc = 0;
    do {
        printf("argv[%d] = %s\n", argc, argv[argc]);
    } while(argv[argc++] != 0);
}
