#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (!s || !out) return -1;
    
    const char *p = s;
    int sign = 1;
    
    // Skip leading whitespace
    while (*p && isspace((unsigned char)*p)) p++;
    
    // Check for sign
    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
        p++;
    }
    
    // Must have at least one digit
    if (!*p || !isdigit((unsigned char)*p)) return -1;
    
    // Parse digits
    int result = 0;
    while (*p && isdigit((unsigned char)*p)) {
        result = result * 10 + (*p - '0');
        p++;
    }
    
    // Skip trailing whitespace only if we've consumed all non-whitespace
    while (*p && isspace((unsigned char)*p)) p++;
    
    // Must have consumed entire string
    if (*p) return -1;
    
    *out = sign * result;
    return 0;
}