#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (!s || !out) return -1;

    int sign = 1;
    long val = 0;
    int started = 0;

    while (*s == ' ') s++; // skip leading spaces (optional, but common)

    if (*s == '-') {
        sign = -1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    while (*s) {
        if (!isdigit((unsigned char)*s)) {
            return -1;
        }
        started = 1;
        val = val * 10 + (*s - '0');
        s++;
    }

    if (!started) return -1; // no digits found

    *out = (int)(sign * val);
    return 0;
}