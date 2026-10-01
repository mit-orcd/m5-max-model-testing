#include <limits.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *p = s;
    int sign = 1;
    long result = 0;
    int valid = 0;

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
        result = result * 10 + (*p - '0');
        valid = 1;
        p++;
    }

    // If no digits were found, it's invalid
    if (!valid) {
        return -1;
    }

    // Apply sign and check for overflow
    if (sign == 1 && result > INT_MAX) {
        return -1;
    }
    if (sign == -1 && -result < INT_MIN) {
        return -1;
    }

    *out = (int)(sign * result);
    return 0;
}