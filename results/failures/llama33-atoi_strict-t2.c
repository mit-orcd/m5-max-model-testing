#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (!s || !*s) return -1;

    int sign = 1;
    if (*s == '-') {
        sign = -1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    int num = 0;
    while (*s && isdigit(*s)) {
        num = num * 10 + (*s - '0');
        s++;
    }

    if (!*s) {
        *out = sign * num;
        return 0;
    }

    return -1;
}