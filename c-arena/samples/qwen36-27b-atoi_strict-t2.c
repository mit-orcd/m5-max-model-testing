#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || *s == '\0' || out == NULL) {
        return -1;
    }

    const char *p = s;
    int sign = 1;
    int result = 0;

    // Handle optional sign
    if (*p == '+') {
        p++;
    } else if (*p == '-') {
        sign = -1;
        p++;
    }

    // Check if there are any digits
    if (!isdigit((unsigned char)*p)) {
        return -1;
    }

    // Parse digits
    while (isdigit((unsigned char)*p)) {
        result = result * 10 + (*p - '0');
        p++;
    }

    // Check if we've reached the end of the string
    if (*p != '\0') {
        return -1;
    }

    *out = sign * result;
    return 0;
}