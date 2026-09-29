#include <ctype.h>
#include <stddef.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) return -1;

    const char *p = s;
    int sign = 1;
    int c;

    /* Skip leading whitespace */
    while (isspace((unsigned char)*p)) p++;

    if (*p == '\0') return -1;

    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
        p++;
    }

    if (*p == '\0') return -1;

    const char *start = p;
    int value = 0;

    /* Parse digits */
    while (isdigit((unsigned char)*p)) {
        int digit = *p - '0';
        value = value * 10 + digit;
        p++;
    }

    if (p == start) return -1; /* No digits found */

    *out = sign * value;

    /* Ensure no trailing characters */
    if (*p != '\0') return -1;

    return 0;
}