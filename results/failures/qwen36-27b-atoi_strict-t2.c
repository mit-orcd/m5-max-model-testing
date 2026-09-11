#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    const char *p = s;
    int sign = 1;

    if (p == NULL || out == NULL) {
        return -1;
    }

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
        p++;
    }

    /* Check for empty string after sign */
    if (*p == '\0') {
        return -1;
    }

    /* Check that the first character is a digit */
    if (!isdigit((unsigned char)*p)) {
        return -1;
    }

    /* Parse digits */
    long value = 0;
    while (isdigit((unsigned char)*p)) {
        value = value * 10 + (*p - '0');
        p++;
    }

    /* Check for trailing non-whitespace characters */
    if (*p != '\0') {
        return -1;
    }

    *out = (int)(value * sign);
    return 0;
}