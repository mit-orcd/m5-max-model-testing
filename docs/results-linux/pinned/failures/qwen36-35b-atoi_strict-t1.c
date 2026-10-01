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
        // The spec says "exactly one valid integer", so trailing chars (including whitespace) make it invalid.
        // However, typically atoi ignores trailing whitespace. But the spec says "trailing chars" is invalid.
        // Let's re-read: "s is not exactly one valid integer (empty, junk, trailing chars)".
        // This implies that if there are any characters after the integer, it's invalid.
        // But what about leading whitespace? The standard atoi skips leading whitespace.
        // The prompt says "Parses an optionally negative decimal integer from s".
        // Usually, strict parsing means no leading/trailing whitespace unless specified.
        // But the example of "trailing chars" being invalid suggests we should check for anything after the number.
        // Let's assume leading whitespace is allowed (like standard atoi) but trailing is not.
        // Actually, let's look at the return condition: "-1 if s is not exactly one valid integer".
        // If we allow leading whitespace, then " 123 " would have trailing space, so invalid.
        // " 123" would be valid.
        // Let's stick to: skip leading whitespace, parse integer, ensure nothing left (or only null terminator).
        
        // Wait, if we skipped leading whitespace, we are at the first digit or sign.
        // After parsing digits, if *p is not '\0', it's invalid.
        return -1;
    }

    *out = sign * result;
    return 0;
}