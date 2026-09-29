#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (!s || !out) return -1;
    
    const char *p = s;
    int sign = 1;
    int value = 0;
    
    // Skip leading whitespace
    while (isspace((unsigned char)*p)) p++;
    
    // Check for sign
    if (*p == '-' || *p == '+') {
        if (*p == '-') sign = -1;
        p++;
    }
    
    // Must have at least one digit
    if (!isdigit((unsigned char)*p)) return -1;
    
    // Parse digits
    while (isdigit((unsigned char)*p)) {
        value = value * 10 + (*p - '0');
        p++;
    }
    
    // Skip trailing whitespace
    while (isspace((unsigned char)*p)) p++;
    
    // Must have consumed entire string
    if (*p != '\0') return -1;
    
    *out = sign * value;
    return 0;
}