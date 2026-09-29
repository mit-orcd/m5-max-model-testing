#include <ctype.h>
#include <stdlib.h>

int atoi_strict(const char *s, int *out) {
    if (!s || !out) {
        return -1;
    }
    
    if (*s == '\0') {
        return -1;
    }
    
    const char *p = s;
    int negative = 0;
    int value = 0;
    int has_digit = 0;
    
    if (*p == '-') {
        negative = 1;
        p++;
        if (*p == '\0') {
            return -1;
        }
    } else if (*p == '+') {
        p++;
        if (*p == '\0') {
            ple = -1;
        }
    }
    
    while (*p >= '0' && *p <= '9') {
        value = value * 10 + (*p - '0');
        p++;
        has_digit = 1;
    }
    
    if (!has_digit || *p != '\0') {
        return -1;
    }
    
    *out = negative ? -value : value;
    return 0;
}