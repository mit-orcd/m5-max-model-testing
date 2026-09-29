#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (!s || !out) return -1;

    int sign = 1;
    long val = 0;
    int started = 0;

    while (*s) {
        if (!started && *s == '-') {
            sign = -1;
            started = 1;
            s++;
            continue;
        }
        if (!started && *s == '+') {
            started = 1;
            s++;
            continue;
        }
        if (isdigit((unsigned char)*s)) {
            started = 1;
            val = val * 10 + (*s - '0');
            s++;
        } else {
            return -1;
        }
    }

    if (!started) return -1;

    *out = (int)(sign * val);
    return 0;
}