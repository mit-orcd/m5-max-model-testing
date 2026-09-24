#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || *s == '\0') {
        return -1;
    }

    int sign = 1;
    int result = 0;

    if (*s == '-') {
        sign = -1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    if (*s == '\0') {
        return -1;
    }

    while (*s != '\0') {
        if (isdigit((unsigned char)*s)) {
            result = result * 10 + (*s - '0');
            s++;
        } else {
            if (*s != '\0') {
                return -1;
            }
            break;
        }
    }

    *out = sign * result;
    return 0;
}