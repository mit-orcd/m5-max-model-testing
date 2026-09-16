#include <ctype.h>
#include <stddef.h>

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

    // Check for sign
    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
        p++;
    }

    // Check if there's at least one digit
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

    // Check that we consumed all non-whitespace characters
    // The string must end here (no trailing non-whitespace)
    if (*p != '\0') {
        return -1;
    }

    *out = sign * result;
    return 0;
}