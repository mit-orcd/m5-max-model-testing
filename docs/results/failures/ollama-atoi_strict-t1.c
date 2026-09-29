```c
#include <stddef.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || *s == '\0')
        return -1;

    int negative = 0;
    const char *p = s;

    if (*p == '-') {
        negative = 1;
        p++;
    }

    if (*p < '0' || *p > '9')
        return -1;

    int value