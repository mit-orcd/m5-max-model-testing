#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    while (isspace((unsigned char)*s)) {
        s++;
    }

    if (*s == '\0') {
        return -1;
    }

    int sign = 1;
    if (*s == '+' || *s == '-') {
        if (*s == '-') {
            sign = -1;
        }
        s++;
    }

    if (*s == '\0') {
        return -1;
    }

    if (!isdigit((unsigned char)*s)) {
        return -1;
    }

    unsigned int value = 0;
    while (isdigit((unsigned char)*s)) {
        value = value * 10 + (*s - '0');
        s++;
    }

    if (sign == 1) {
        *out = value;
    } else {
        *out = -value;
    }

    if (*s != '\0') {
        return -1;
    }

    return 0;
}