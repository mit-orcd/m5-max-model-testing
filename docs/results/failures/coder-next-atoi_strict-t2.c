#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (!s || !out) return -1;
    
    const char *p = s;
    int sign = 1;
    
    // Skip leading whitespace
    while (*p && isspace((unsigned char)*p)) {
        p++;
    }
    
    // Check for sign
    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
        p++;
    }
    
    // Must have at least one digit
    if (!*p || !isdigit((unsigned char)*p)) {
        return -1;
    }
    
    int value = 0;
    
    // Parse digits
    while (isdigit((unsigned char)*p)) {
        value = value * 10 + (*p - '0');
        p++;
    }
    
    // Skip trailing whitespace
    while (*p && isspace((unsigned char)*p)) {
        p++;
    }
    
    // Must be at end of string
    if (*p != '\0') {
        return -1;
    }
    
    *out = sign * value;
    return 0;
}