#include <stddef.h>
#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    const char *p = s;
    int sign = 1;
    int value = 0;

    if (s == NULL) {
        return -1;
    }

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

    /* Check for empty string after sign */
    if (*p == '\0') {
        return -1;
    }

    /* Parse digits */
    while (isdigit((unsigned char)*p)) {
        value = value * 10 + (*p - '0');
        p++;
    }

    /* Ensure no trailing characters */
    if (*p != '\0') {
        return -1;
    }

    *out = sign * value;
    return 0;
}