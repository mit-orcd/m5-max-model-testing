#include <ctype.h>
#include <limits.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *p = s;
    
    // Skip leading whitespace
    while (isspace((unsigned char)*p)) {
        p++;
    }

    // Check for empty string after whitespace
    if (*p == '\0') {
        return -1;
    }

    // Handle optional sign
    int sign = 1;
    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
        p++;
    }

    // Must have at least one digit
    if (!isdigit((unsigned char)*p)) {
        return -1;
    }

    long long val = 0;
    while (isdigit((unsigned char)*p)) {
        val = val * 10 + (*p - '0');
        p++;
    }

    // Check for trailing non-whitespace characters
    if (*p != '\0') {
        return -1;
    }

    // Apply sign
    if (sign == -1) {
        val = -val;
    }

    // No overflow tests required per spec, but ensure it fits in int for assignment
    // Since spec says "No overflow tests", we assume input fits or behavior is undefined.
    // However, to safely assign to int*, we cast.
    *out = (int)val;

    return 0;
}