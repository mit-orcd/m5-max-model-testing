#include <stdbool.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *p = s;
    bool negative = false;
    int value = 0;
    bool has_digits = false;

    if (*p == '+') {
        p++;
    } else if (*p == '-') {
        negative = true;
        p++;
    }

    if (*p < '0' || *p > '9') {
        return -1;
    }

    while (*p >= '0' && *p <= '9') {
        value = value * 10 + (*p - '0');
        has_digits = true;
        p++;
    }

    if (*p != '\0') {
        return -1;
    }

    if (negative) {
        value = -value;
    }

    *out = value;
    return 0;
}