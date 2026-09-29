#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL || *s == '\0') {
        return -1;
    }

    int sign = 1;
    if (*s == '-') {
        sign = -1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    if (*s == '\0') {
        return -1;
    }

    char *digits_start = (char *)s;
    while (*s != '\0') {
        if (!isdigit((unsigned char)*s)) {
            return -1;
        }
        s++;
    }

    int result = 0;
    while (digits_start < s) {
        result = result * 10 + (*digits_start - '0');
        digits_start++;
    }

    *out = sign * result;
    return 0;
}