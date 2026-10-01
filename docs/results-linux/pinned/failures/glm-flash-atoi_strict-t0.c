#include <limits.h>
#include <stdbool.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *p = s;
    bool negative = false;

    // Skip leading whitespace
    while (*p == ' ') {
        p++;
    }

    // Handle optional sign
    if (*p == '-') {
        negative = true;
        p++;
    } else if (*p == '+') {
        p++;
    }

    // Check for empty string after sign
    if (*p == '\0') {
        return -1;
    }

    // Parse digits
    long result = 0;
    bool has_digits = false;

    while (*p >= '0' && *p <= '9') {
        has_digits = true;
        result = result * 10 + (*p - '0');
        p++;
    }

    // Must have at least one digit
    if (!has_digits) {
        return -1;
    }

    // Check for trailing characters
    while (*p != '\0') {
        if (*p != ' ') {
            return -1;
        }
        p++;
    }

    // Apply sign and check bounds
    if (negative) {
        if (result > (unsigned long)INT_MAX + 1) {
            return -1;
        }
        *out = -(int)result;
    } else {
        if (result > INT_MAX) {
            return -1;
        }
        *out = (int)result;
    }

    return 0;
}