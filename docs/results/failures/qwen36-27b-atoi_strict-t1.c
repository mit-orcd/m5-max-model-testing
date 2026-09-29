#include <ctype.h>
#include <stddef.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *p = s;
    int sign = 1;
    int result = 0;
    int has_digit = 0;

    /* Skip leading whitespace */
    while (isspace((unsigned char)*p)) {
        p++;
    }

    /* Check for sign */
    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
        sign = 1;
        p++;
    }

    /* Parse digits */
    while (isdigit((unsigned char)*p)) {
        result = result * 10 + (*p - '0');
        p++;
        has_digit = 1;
    }

    /* Check if we parsed at least one digit and nothing remains */
    if (!has_digit || *p != '\0') {
        return -1;
    }

    *out = sign * result;
    return 0;
}