#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (!s || !out) return -1;
    
    const char *p = s;
    int sign = 1;
    
    // Skip leading whitespace
    while (isspace((unsigned char)*p)) p++;
    
    // Handle sign
    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
        p++;
    }
    
    // Must have at least one digit
    if (!isdigit((unsigned char)*p)) return -1;
    
    int result = 0;
    while (isdigit((unsigned char)*p)) {
        result = result * 10 + (*p - '0');
        p++;
    }
    
    // Skip trailing whitespace
    while (isspace((unsigned char)*p)) p++;
    
    // Must have consumed entire string
    if (*p != '\0') return -1;
    
    *out = sign * result;
    return 0;
}