#include <ctype.h>
#include <stddef.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) return -1;

    // Skip leading whitespace
    while (isspace((unsigned char)*s)) s++;

    // Check for empty string
    if (*s == '\0') return -1;

    // Handle sign
    int sign = 1;
    if (*s == '-') {
        sign = -1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    // Check if there's at least one digit
    if (!isdigit((unsigned char)*s)) return -1;

    // Parse digits
    long long value = 0;
    while (isdigit((unsigned char)*s)) {
        value = value * 10 + (*s - '0');
        s++;
    }

    // Check for trailing non-digit characters
    while (isspace((unsigned char)*s)) s++;
    if (*s != '\0') return -1;

    *out = sign * value;
    return 0;
}