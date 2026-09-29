#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || *s == '\0') {
        return -1;
    }
    
    int sign = 1;
    int i = 0;
    
    if (s[0] == '-') {
        sign = -1;
        i = 1;
    } else if (s[0] == '+') {
        i = 1;
    }
    
    int result = 0;
    int first_digit = 0;
    
    for (; s[i] != '\0'; i++) {
        if (!isdigit(s[i])) {
            return -1;
        }
        first_digit = 1;
        result = result * 10 + (s[i] - '0');
    }
    
    if (!first_digit) {
        return -1;
    }
    
    *out = result * sign;
    return 0;
}