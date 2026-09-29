#include <stddef.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *p = s;
    int negative = 0;

    /* Check for optional minus sign */
    if (*p == '-') {
        negative = 1;
        p++;
    }

    /* Must have at least one digit */
    if (*p < '0' || *p > '9') {
        return -1;
    }

    long long val = 0;
    while (*p >= '0' && *p <= '9') {
        val = val * 10 + (*p - '0');
        p++;
    }

    /* Must be at end of string (no trailing characters) */
    if (*p != '\0') {
        return -1;
    }

    if (negative) {
        val = -val;
    }

    /* Check for overflow beyond int range */
    if (val > INT_MAX || val < INT_MIN) {
        return -1;
    }

    *out = (int)val;
    return 0;
}