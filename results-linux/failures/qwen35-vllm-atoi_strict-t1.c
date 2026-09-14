#include <ctype.h>
#include <limits.h>

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

    if (*p == '\0') {
        return -1;
    }

    while (isdigit((unsigned char)*p)) {
        p++;
    }

    if (*p != '\0') {
        return -1;
    }

    long long val = 0;
    const char *temp = s;
    if (*temp == '-' || *temp == '+') temp++;
    
    while (isdigit((unsigned char)*temp)) {
        val = val * 10 + (*temp - '0');
        temp++;
    }

    *out = (int)(val * sign);
    return 0;
}