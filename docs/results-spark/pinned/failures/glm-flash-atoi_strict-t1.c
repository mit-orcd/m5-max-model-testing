#include <limits.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *p = s;
    int sign = 1;
    long value = 0;
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
        valid = 1;
        value = value * 10 + (*p - '0');
        p++;
    }

    // If no digits were found, it's invalid
    if (!valid) {
        return -1;
    }

    // Apply sign
    value *= sign;

    // Check for overflow (since we used long)
    if (value < INT_MIN || value > INT_MAX) {
        return -1;
    }

    // Check for trailing characters
    while (*p != '\0') {
        if (*p != ' ') {
            return -1;
        }
        p++;
    }

    *out = (int)value;
    return 0;
}