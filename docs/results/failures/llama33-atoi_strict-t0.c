#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    int sign = 1;
    int num = 0;

    if (*s == '-') {
        sign = -1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    while (*s && isdigit(*s)) {
        num = num * 10 + (*s - '0');
        s++;
    }

    if (s == NULL || *s != '\0' || (sign == 1 && *s == '-') || (sign == -1 && *s == '+')) {
        return -1;
    }

    if (num == 0 && *s == '\0') {
        *out = sign * num;
        return 0;
    }

    if (*s != '\0') {
        return -1;
    }

    *out = sign * num;
    return 0;
}