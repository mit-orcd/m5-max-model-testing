#include <ctype.h>
#include <limits.h>

int atoi_strict(const char *s, int *out) {
    const char *p = s;
    int sign = 1;
    long val = 0;

    while (isspace((unsigned char)*p)) {
        p++;
    }

    if (*p == '+') {
        p++;
    } else if (*p == '-') {
        sign = -1;
        p++;
    }

    if (!isdigit((unsigned char)*p)) {
        return -1;
    }

    while (isdigit((unsigned char)*p)) {
        val = val * 10 + (*p - '0');
        p++;
    }

    if (*p != '\0') {
        return -1;
    }

    if (val > INT_MAX) {
        return -1;
    }

    *out = sign * val;
    return 0;
}