```c
#include <stddef.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    const char *p = s;

    while (p && *p && count < max) {
        int sign = 1;
        int value = 0;

        if (*p == '-') {
            sign = -1;
            p++;
        }

        while (*p >= '0' && *p <= '9') {
            value = value * 10 + (*p - '0');
            p++;
        }

        out[count++] = sign * value;

        if