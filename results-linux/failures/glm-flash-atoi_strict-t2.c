#include <limits.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *p = s;

    // Skip leading whitespace
    while (*p == ' ') {
        p++;
    }

    // Handle sign
    int negative = 0;
    if (*p == '-') {
        negative = 1;
        p++;
    } else if (*p == '+') {
        p++;
    }

    // At least one digit required
    if (*p < '0' || *p > '9') {
        return -1;
    }

    long result = 0;
    int digit;

    // Convert digits
    while (*p != '\0') {
        digit = *p - '0';
        if (digit < 0 || digit > 9) {
            break;
        }

        result = result * 10 + digit;

        // Check for overflow before it happens
        if (result > INT_MAX) {
            return -1;
        }

        p++;
    }

    // Ensure we consumed the entire string
    // (No trailing whitespace allowed)
    while (*p != '\0') {
        return -1;
    }

    *out = (int)result;
    if (negative) {
        *out = -*out;
    }

    return 0;
}