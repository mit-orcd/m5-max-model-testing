#include <ctype.h>
#include <stddef.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *p = s;
    int sign = 1;
    int value = 0;

    /* Skip leading whitespace */
    while (isspace((unsigned char)*p)) {
        p++;
    }

    /* Check for empty string */
    if (*p == '\0') {
        return -1;
    }

    /* Handle optional sign */
    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
        sign = 1;
        p++;
    }

    /* Check for empty string after sign */
    if (*p == '\0') {
        return -1;
    }

    /* Parse digits */
    if (!isdigit((unsigned char)*p)) {
        return -1;
    }

    while (isdigit((unsigned char)*p)) {
        value = value * 10 + (*p - '0');
        p++;
    }

    /* Check for trailing characters (must be end of string) */
    if (*p != '\0') {
        return -1;
    }

    *out = sign * value;
    return 0;
}