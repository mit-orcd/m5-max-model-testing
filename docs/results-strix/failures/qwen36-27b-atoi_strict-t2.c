#include <stddef.h>
#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *p = s;
    int sign = 1;
    int value = 0;
    int has_digit = 0;

    // Skip leading whitespace
    while (isspace((unsigned char)*p)) {
        p++;
    }

    // Check for optional sign
    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
        sign = 1;
        p++;
    }

    // Parse digits
    while (isdigit((unsigned char)*p)) {
        value = value * 10 + (*p - '0');
        p++;
        has_digit = 1;
    }

    // Ensure we parsed at least one digit and nothing remains but whitespace
    if (!has_digit) {
        return -1;
    }

    // Skip trailing whitespace
    while (isspace((unsigned char)*p)) {
        p++;
    }

    // Ensure string ends here
    if (*p != '\0') {
        return -1;
    }

    *out = sign * value;
    return 0;
}