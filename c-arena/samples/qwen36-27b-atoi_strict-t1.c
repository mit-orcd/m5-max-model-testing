#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *start = s;
    int sign = 1;
    long acc = 0;

    // Skip leading whitespace
    while (*s && isspace((unsigned char)*s)) {
        s++;
    }

    // Check for empty string
    if (*s == '\0') {
        return -1;
    }

    // Handle sign
    if (*s == '-') {
        sign = -1;
        s++;
    } else if (*s == '+') {
        sign = 1;
        s++;
    }

    // Check for empty after sign
    if (*s == '\0') {
        return -1;
    }

    // Parse digits
    if (!isdigit((unsigned char)*s)) {
        return -1;
    }

    while (*s && isdigit((unsigned char)*s)) {
        acc = acc * 10 + (*s - '0');
        s++;
    }

    // Check for trailing characters
    if (*s != '\0') {
        return -1;
    }

    *out = (int)(sign * acc);
    return 0;
}