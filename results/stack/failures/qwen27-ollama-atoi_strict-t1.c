```c
#include <stddef.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL)
        return -1;

    int neg = 0;
    if (*s == '-') {