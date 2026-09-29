#include <limits.h>
#include <stdbool.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *p = s;
    bool negative = false;

    // Check for empty string
    if (*p == '\0') {
        return -1;
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
    while (*p >= '0' && *p <= '9') {
        result = result * 10 + (*p - '0');
        p++;
    }

    // Check for trailing characters
    if (*p != '\0') {
        return -1;
    }

    // Apply sign and check for overflow
    if (negative) {
        if (result > INT_MAX + 1L) {
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