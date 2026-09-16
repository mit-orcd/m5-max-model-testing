#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (!s || !out) return -1;

    const char *p = s;
    int sign = 1;
    long long result = 0;

    // Skip leading whitespace
    while (isspace((unsigned char)*p)) p++;

    // Check for sign
    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
        p++;
    }

    // Must have at least one digit
    if (!isdigit((unsigned char)*p)) return -1;

    // Parse digits
    while (isdigit((unsigned char)*p)) {
        result = result * 10 + (*p - '0');
        p++;
    }

    // No trailing characters allowed
    if (*p != '\0') return -1;

    *out = (int)(sign * result);
    return 0;
}