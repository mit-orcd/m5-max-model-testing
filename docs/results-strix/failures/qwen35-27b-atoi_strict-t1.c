#include <ctype.h>
#include <stddef.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *p = s;
    int sign = 1;
    int value = 0;
    int has_digits = 0;

    /* Skip leading whitespace */
    while (isspace((unsigned char)*p)) {
        p++;
    }

    /* Check for sign */
    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
        p++;
    }

    /* Parse digits */
    while (isdigit((unsigned char)*p)) {
        value = value * 10 + (*p - '0');
        p++;
        has_digits = 1;
    }

    /* Ensure the string ended exactly after the number */
    if (!has_digits || *p != '\0') {
        return -1;
    }

    *out = sign * value;
    return 0;
}