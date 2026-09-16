#include <limits.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *p = s;
    int negative = 0;

    /* Optionally negative */
    if (*p == '-') {
        negative = 1;
        p++;
    }

    /* Must have at least one digit */
    if (*p < '0' || *p > '9') {
        return -1;
    }

    int value = 0;
    while (*p >= '0' && *p <= '9') {
        value = value * 10 + (*p - '0');
        p++;
    }

    /* No trailing characters allowed */
    if (*p != '\0') {
        return -1;
    }

    if (negative) {
        value = -value;
    }

    *out = value;
    return 0;
}