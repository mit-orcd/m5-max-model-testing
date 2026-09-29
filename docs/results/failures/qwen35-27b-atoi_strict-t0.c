#include <ctype.h>
#include <stddef.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *p = s;
    int sign = 1;
    int value = 0;
    int has_digit = 0;

    /* Skip leading whitespace */
    while (isspace((unsigned char)*p)) {
        p++;
    }

    /* Check for empty string after whitespace */
    if (*p == '\0') {
        return -1;
    }

    /* Handle optional sign */
    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
        p++;
    }

    /* Parse digits */
    while (isdigit((unsigned char)*p)) {
        has_digit = 1;
        value = value * 10 + (*p - '0');
        p++;
    }

    /* Must have at least one digit and no trailing characters */
    if (!has_digit || *p != '\0') {
        return -1;
    }

    *out = sign * value;
    return 0;
}