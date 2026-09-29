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

    // Handle optional sign
    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
        p++;
    }

    // Check if there is at least one digit
    if (!isdigit((unsigned char)*p)) {
        return -1;
    }

    // Parse digits
    int result = 0;
    while (isdigit((unsigned char)*p)) {
        int digit = *p - '0';
        result = result * 10 + digit;
        p++;
    }

    // Check for trailing non-whitespace characters
    if (*p != '\0') {
        // Allow trailing whitespace only if it was part of the initial skip? 
        // The spec says "exactly one valid integer", so trailing chars (including whitespace) after the number are invalid.
        // However, typically atoi allows trailing whitespace. But the spec says "trailing chars" are invalid.
        // Let's re-read: "s is not exactly one valid integer (empty, junk, trailing chars)".
        // This implies that after the integer, there should be nothing (or maybe just end of string).
        // Standard atoi ignores trailing whitespace. But "strict" usually means no trailing chars.
        // Let's assume trailing whitespace is NOT allowed based on "trailing chars" being invalid.
        // Actually, let's look at common strict implementations. Often they require the entire string to be the number.
        // But leading whitespace is usually allowed. Trailing whitespace is often not.
        // Let's stick to: after parsing digits, the next char must be '\0'.
        return -1;
    }

    *out = sign * result;
    return 0;
}