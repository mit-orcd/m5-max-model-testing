#include <limits.h>
#include <stdbool.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *p = s;
    bool negative = false;

    // Skip leading whitespace
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\v' || *p == '\f' || *p == '\r') {
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

    int result = 0;
    bool has_digits = false;

    // Parse digits
    while (*p >= '0' && *p <= '9') {
        int digit = *p - '0';
        result = result * 10 + digit;
        p++;
        has_digits = true;
    }

    // If no digits were parsed, it's invalid
    if (!has_digits) {
        return -1;
    }

    // Apply sign
    if (negative) {
        result = -result;
    }

    // Handle overflow (simple check)
    if (negative && result == INT_MIN) {
        return -1;
    }
    if (!negative && result == INT_MAX) {
        return -1;
    }

    // Ensure no trailing characters remain
    if (*p != '\0') {
        return -1;
    }

    *out = result;
    return 0;
}