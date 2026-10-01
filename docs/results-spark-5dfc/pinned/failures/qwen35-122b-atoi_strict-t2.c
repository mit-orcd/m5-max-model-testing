#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *p = s;
    int sign = 1;
    int has_digit = 0;
    int result = 0;

    // Skip leading whitespace
    while (*p != '\0' && isspace((unsigned char)*p)) {
        p++;
    }

    // Check for empty string after whitespace
    if (*p == '\0') {
        return -1;
    }

    // Handle optional sign
    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
        p++;
    }

    // Parse digits
    while (*p != '\0' && isdigit((unsigned char)*p)) {
        has_digit = 1;
        result = result * 10 + (*p - '0');
        p++;
    }

    // Must have at least one digit
    if (!has_digit) {
        return -1;
    }

    // Must end immediately after the number (no trailing junk)
    if (*p != '\0') {
        return -1;
    }

    *out = sign * result;
    return 0;
}