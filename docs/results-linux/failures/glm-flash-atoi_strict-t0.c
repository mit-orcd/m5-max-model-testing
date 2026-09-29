#include <limits.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *p = s;
    int sign = 1;
    long value = 0;

    // Skip leading whitespace
    while (*p == ' ') {
        p++;
    }

    // Handle optional sign
    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
        p++;
    }

    // Parse digits
    while (*p >= '0' && *p <= '9') {
        value = value * 10 + (*p - '0');
        p++;
    }

    // Check for trailing characters or empty input
    if (*p != '\0') {
        return -1;
    }

    // Apply sign and check for overflow
    if (sign == 1 && value > INT_MAX) {
        return -1;
    }
    if (sign == -1 && -value < INT_MIN) {
        return -1;
    }

    *out = (int)(sign * value);
    return 0;
}