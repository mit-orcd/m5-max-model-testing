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

    // Check for empty string after whitespace
    if (*p == '\0') {
        return -1;
    }

    // Handle sign
    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
        p++;
    }

    // Must have at least one digit after sign
    if (*p == '\0' || !isdigit((unsigned char)*p)) {
        return -1;
    }

    // Parse digits
    int result = 0;
    while (isdigit((unsigned char)*p)) {
        int digit = *p - '0';
        result = result * 10 + digit;
        p++;
    }

    // Apply sign
    if (sign == -1) {
        result = -result;
    }

    // Check that we consumed all non-whitespace characters after the number
    // The problem says "no overflow tests" and "exactly one valid integer"
    // We need to check for trailing non-whitespace characters
    // But the spec says "trailing chars" should return -1
    // However, since we can't detect overflow, we just check for trailing non-whitespace
    // Actually, let's re-read: "s is not exactly one valid integer (empty, junk, trailing chars)"
    // This implies that after the integer, there should be nothing or just end of string?
    // Or should we allow trailing whitespace? Typically atoi allows trailing whitespace.
    // But "trailing chars" suggests non-whitespace trailing characters are invalid.
    // Let's allow trailing whitespace as that's standard for atoi-like functions.
    
    // Check for trailing non-whitespace
    while (isspace((unsigned char)*p)) {
        p++;
    }
    
    if (*p != '\0') {
        return -1;
    }

    *out = result;
    return 0;
}