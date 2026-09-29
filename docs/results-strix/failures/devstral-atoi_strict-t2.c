#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) return -1;

    // Skip leading whitespace
    while (isspace((unsigned char)*s)) s++;

    // Check for empty string or non-digit start
    if (*s == '\0' || (!isdigit((unsigned char)*s) && *s != '-')) return -1;

    // Handle negative sign
    int sign = 1;
    if (*s == '-') {
        sign = -1;
        s++;
        // After '-', must have a digit
        if (!isdigit((unsigned char)*s)) return -1;
    }

    // Parse digits
    long long num = 0;
    while (isdigit((unsigned char)*s)) {
        num = num * 10 + (*s - '0');
        s++;
    }

    // Check for trailing characters
    while (isspace((unsigned char)*s)) s++;
    if (*s != '\0') return -1;

    *out = sign * num;
    return 0;
}