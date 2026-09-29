#include <ctype.h>
#include <limits.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *p = s;
    int sign = 1;
    long result = 0;
    int overflow = 0;

    // Skip leading whitespace
    while (isspace((unsigned char)*p)) {
        p++;
    }

    // Handle optional sign
    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
        p++;
    }

    // Parse digits
    while (*p != '\0') {
        if (!isdigit((unsigned char)*p)) {
            return -1;
        }
        
        int digit = *p - '0';
        
        // Simple overflow check to ensure result fits in a long
        // before final conversion to int
        if (result > (LONG_MAX - digit) / 10) {
            overflow = 1;
        }
        
        result = result * 10 + digit;
        p++;
    }

    // Reject empty strings (no digits found)
    if (p == s) {
        return -1;
    }

    // Reject overflow
    if (overflow) {
        return -1;
    }

    // Convert to int
    *out = (int)(sign * result);
    return 0;
}