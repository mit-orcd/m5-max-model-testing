#include <stddef.h>
#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *p = s;

    /* Skip leading whitespace */
    while (isspace((unsigned char)*p)) {
        p++;
    }

    /* Check for empty string after whitespace */
    if (*p == '\0') {
        return -1;
    }

    /* Handle optional sign */
    int sign = 1;
    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
        p++;
    }

    /* Check for digit after sign */
    if (!isdigit((unsigned char)*p)) {
        return -1;
    }

    /* Parse digits */
    long result = 0;
    while (isdigit((unsigned char)*p)) {
        result = result * 10 + (*p - '0');
        p++;
    }

    /* Check for trailing non-whitespace characters */
    if (*p != '\0') {
        return -1;
    }

    *out = (int)(result * sign);
    return 0;
}