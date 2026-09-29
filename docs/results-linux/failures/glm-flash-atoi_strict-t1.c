#include <limits.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || *s == '\0') {
        return -1;
    }

    int sign = 1;
    const char *p = s;

    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
        p++;
    }

    if (*p == '\0') {
        return -1;
    }

    long value = 0;
    int overflow = 0;

    while (*p != '\0') {
        if (*p < '0' || *p > '9') {
            return -1;
        }

        int digit = *p - '0';

        if (value > (LONG_MAX - digit) / 10) {
            overflow = 1;
        }

        value = value * 10 + digit;
        p++;
    }

    if (overflow) {
        return -1;
    }

    if (value > INT_MAX) {
        return -1;
    }

    *out = (int)(value * sign);
    return 0;
}