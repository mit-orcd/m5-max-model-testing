#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (!s || !out) return -1;

    // Skip leading whitespace
    while (isspace((unsigned char)*s)) s++;

    // Check for empty string
    if (*s == '\0') return -1;

    // Handle sign
    int sign = 1;
    if (*s == '+') {
        s++;
    } else if (*s == '-') {
        sign = -1;
        s++;
    }

    // Check for digits
    if (!isdigit((unsigned char)*s)) return -1;

    long long result = 0;
    while (isdigit((unsigned char)*s)) {
        result = result * 10 + (*s - '0');
        s++;
    }

    // Check for trailing non-digits
    if (*s != '\0') return -1;

    *out = sign * result;
    return 0;
}