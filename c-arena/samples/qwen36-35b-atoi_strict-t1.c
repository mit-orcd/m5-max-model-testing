#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) return -1;

    const char *p = s;
    int negative = 0;

    // Skip leading whitespace
    while (isspace((unsigned char)*p)) p++;

    // Check for empty string after whitespace
    if (*p == '\0') return -1;

    // Handle sign
    if (*p == '-') {
        negative = 1;
        p++;
    } else if (*p == '+') {
        p++;
    }

    // Check if there are any digits following the sign
    if (*p == '\0' || !isdigit((unsigned char)*p)) return -1;

    // Parse digits
    int result = 0;
    while (isdigit((unsigned char)*p)) {
        int digit = *p - '0';
        result = result * 10 + digit;
        p++;
    }

    if (negative) result = -result;

    *out = result;
    return 0;
}