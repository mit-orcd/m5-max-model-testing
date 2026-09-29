#include <stdint.h>

int atoi_strict(const char *s, int *out) {
    if (!s || !out) return -1;

    int sign = 1;
    long value = 0;
    int started = 0;

    while (*s) {
        if (*s == '-' && !started) {
            sign = -1;
            started = 1;
        } else if (*s >= '0' && *s <= '9') {
            started = 1;
            value = value * 10 + (*s - '0');
        } else {
            return -1;
        }
        s++;
    }

    if (!started) return -1;

    *out = (int)(sign * value);
    return 0;
}