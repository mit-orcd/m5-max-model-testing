#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *p = s;
    
    // Skip leading whitespace is NOT allowed for "strict" parsing of "exactly one integer"
    // unless the integer itself starts with whitespace? No, standard atoi skips whitespace.
    // However, "strict" usually implies no leading whitespace unless specified.
    // Let's assume standard integer format: optional sign, then digits.
    // If there is leading whitespace, it's "junk" before the number -> fail.
    
    if (*p == '\0') {
        return -1; // Empty string
    }

    int sign = 1;
    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
        p++;
    }

    if (!isdigit((unsigned char)*p)) {
        return -1; // No digits found after optional sign
    }

    long long val = 0;
    while (isdigit((unsigned char)*p)) {
        val = val * 10 + (*p - '0');
        p++;
    }

    // Check for trailing characters
    if (*p != '\0') {
        return -1;
    }

    // Apply sign
    if (sign == -1) {
        val = -val;
    }

    // No overflow tests required per spec, but we must ensure it fits in int?
    // Spec says "No overflow tests", implying we just cast.
    *out = (int)val;

    return 0;
}