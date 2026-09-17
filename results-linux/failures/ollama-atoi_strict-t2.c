```c
#include <stddef.h>

int atoi_strict(const char *s, int *out)
{
    if (s == NULL || out == NULL)
        return -1;

    int neg = 0;
    const char *p = s;

    if (*p == '-') {
        neg = 1;
        p++;
    }

    if (*p < '0' || *p > '9')
        return -1;

    int val = 0;
    while (*p >= '0'