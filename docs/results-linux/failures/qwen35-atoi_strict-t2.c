#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (!s || !out) return -1;

    while (isspace((unsigned char)*s)) {
        s++;
    }

    if (*s == '\0') return -1;

    int sign = 1;
    if (*s == '-') {
        sign = -1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    if (!isdigit((unsigned char)*s)) return -1;

    int result = 0;
    while (isdigit((unsigned char)*s)) {
        result = result * 10 + (*s - '0');
        s++;
    }

    if (*s != '\0') return -1;

    *out = sign * result;
    return 0;
}