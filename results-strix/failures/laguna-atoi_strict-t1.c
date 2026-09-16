#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (!s || !out) return -1;
    
    int sign = 1;
    int result = 0;
    
    // Skip leading whitespace
    while (isspace((unsigned char)*s)) s++;
    
    // Check for empty string
    if (*s == '\0') return -1;
    
    // Handle optional sign
    if (*s == '-') {
        sign = -1;
        s++;
    } else if (*s == '+') {
        s++;
    }
    
    // Must have at least one digit
    if (!isdigit((unsigned char)*s)) return -1;
    
    // Parse digits
    while (isdigit((unsigned char)*s)) {
        result = result * 10 + (*s - '0');
        s++;
    }
    
    // No trailing characters allowed
    if (*s != '\0') return -1;
    
    *out = sign * result;
    return 0;
}