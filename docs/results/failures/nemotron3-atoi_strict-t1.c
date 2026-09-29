#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (!s || !out) return -1;

    int sign = 1;
    long val = 0;
    int started = 0;

    while (*s && isspace(*s)) s++;

    if (*s == '-') {
        sign = -1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    while (*s) {
        if (!isdigit(*s)) {
            return -1;
        }
        started = 1;
        val = val * 10 + (*s - '0');
        s++;
    }

    if (!started) return -1;

    *out = sign * (int)val;
    return 0;
}