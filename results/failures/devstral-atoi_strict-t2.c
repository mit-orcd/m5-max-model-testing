#include <stdint.h>

int atoi_strict(const char *s, int *out) {
    const char *p = s;
    int sign = 1;
    if (*p == '-') {
        sign = -1;
        p++;
    }
    if (*p == '0') {
        p++;
        if (*p != '\0') return -1;
        *out = 0;
        return 0;
    }
    uint32_t value = 0;
    while (*p >= '1' && *p <= '9') {
        value = value * 10 + (*p - '0');
        p++;
    }
    if (*p != '\0') return -1;
    *out = sign * value;
    return 0;
}