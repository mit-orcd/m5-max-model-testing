#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *p = s;
    int sign = 1;

    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
        p++;
    }

    if (!isdigit((unsigned char)*p)) {
        return -1;
    }

    const char *digits_start = p;
    while (isdigit((unsigned char)*p)) {
        p++;
    }

    if (p == digits_start || *p != '\0') {
        return -1;
    }

    long long value = 0;
    const char *curr = digits_start;
    while (*curr != '\0') {
        value = value * 10 + (*curr - '0');
        curr++;
    }

    value *= sign;
    *out = (int)value;
    return 0;
}