#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *p = s;

    // Skip leading whitespace? Strict usually implies no leading whitespace unless specified.
    // The prompt says "optionally negative decimal integer", implying the string must start with optional '-' or digit.
    // If there is leading whitespace, it's considered "junk" or not "exactly one valid integer" in strict parsing.
    // However, standard atoi ignores whitespace. "Strict" usually means no whitespace allowed at start.
    // Let's assume strict means: optional sign followed immediately by digits, nothing else.

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

    *out = (int)(val * sign);
    return 0;
}