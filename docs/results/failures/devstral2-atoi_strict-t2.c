#include <ctype.h>
#include <limits.h>

int atoi_strict(const char *s, int *out) {
    const char *p = s;
    int sign = 1;
    int value = 0;

    // Skip leading whitespace
    while (*p == ' ') p++;
    if (*p == '\0') return -1;

    // Handle sign
    if (*p == '+') {
        p++;
    } else if (*p == '-') {
        sign = -1;
        p++;
    }

    // Parse digits
    if (!isdigit(*p)) return -1;
    while (isdigit(*p)) {
        int digit = *p - '0';
        value = value * 10 + digit;
        p++;
    }

    // Check for trailing characters
    while (*p == ' ') p++;
    if (*p != '\0') return -1;

    *out = sign * value;
    return 0;
}