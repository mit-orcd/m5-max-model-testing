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
    while (*p != '\0') {
        if (*p < '0' || *p > '9') {
            return -1;
        }

        // Check for overflow before adding the next digit
        if (result > (LONG_MAX - (*p - '0')) / 10) {
            return -1;
        }

        result = result * 10 + (*p - '0');
        p++;
    }

    // Convert to int
    *out = (int)result;

    // Check for overflow into negative range
    if (negative && result > INT_MAX) {
        return -1;
    }

    return 0;
}