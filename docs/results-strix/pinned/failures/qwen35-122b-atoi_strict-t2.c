#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *p = s;
    int sign = 1;
    int val = 0;
    int has_digit = 0;

    if (*p == '\0') {
        return -1;
    }

    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
        p++;
    }

    if (*p == '\0') {
        return -1;
    }

    while (*p != '\0') {
        if (!isdigit((unsigned char)*p)) {
            return -1;
        }
        val = val * 10 + (*p - '0');
        has_digit = 1;
        p++;
    }

    if (!has_digit) {
        return -1;
    }

    *out = sign * val;
    return 0;
}