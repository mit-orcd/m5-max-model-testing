#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || *s == '\0') {
        return -1;
    }

    int sign = 1;
    if (*s == '-') {
        sign = -1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    int value = 0;
    while (*s != '\0') {
        if (isdigit((unsigned char)*s)) {
            value = value * 10 + (*s - '0');
            s++;
        } else {
            return -1;
        }
    }

    *out = sign * value;
    return 0;
}