```c
#include <limits.h>

void itoa(int value, char *buf)
{
    unsigned long long u;
    int neg = 0;
    if (value < 0) {
        neg = 1;
        u = -(unsigned long long)value;