```c
#include <stddef.h>

int atoi_strict(const char *s, int *out)
{
    if (s == NULL || out == NULL)
        return -1;

    if (*s == '\0')
        return -1;

    int neg = 0;
    if (*s == '-') {
        neg = 1;
        s++;
    }

    if (*s < '0' || *s > '9')
        return -1;

    int val = 0;
    while (*s >= '0' && *s <= '9') {
        val = val * 10 + (*s - '0