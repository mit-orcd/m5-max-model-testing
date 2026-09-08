#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *p = s;
    int sign = 1;

    // Skip leading whitespace
    while (isspace((unsigned char)*p)) {
        p++;
    }

    // Check for optional sign
    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
        p++;
    }

    // Check if we have digits
    if (*p < '0' || *p > '9') {
        return -1;
    }

    // Parse digits
    int result = 0;
    while (*p >= '0' && *p <= '9') {
        // Simple digit accumulation without overflow check as per spec
        result = result * 10 + (*p - '0');
        p++;
    }

    // Check that we consumed the entire string (only whitespace allowed after)
    // But per spec: "s is not exactly one valid integer (empty, junk, trailing chars)"
    // This implies no trailing characters except possibly none.
    // Let's check if p points to null terminator only.
    // If there are trailing non-whitespace characters, it's invalid.
    // However, typical atoi_strict implementations might allow trailing whitespace.
    // The spec says: "trailing chars" is invalid. So we check for exactly the integer.
    
    // If we stopped at null terminator, it's valid.
    // If we stopped at a non-null character, it's invalid.
    if (*p != '\0') {
        return -1;
    }

    *out = result * sign;
    return 0;
}