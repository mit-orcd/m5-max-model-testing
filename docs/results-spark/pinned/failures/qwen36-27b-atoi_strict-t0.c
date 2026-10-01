#include <ctype.h>
#include <stddef.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *p = s;
    int sign = 1;
    long long val = 0;

    // Skip leading whitespace
    while (isspace((unsigned char)*p)) {
        p++;
    }

    // Check for empty string after whitespace
    if (*p == '\0') {
        return -1;
    }

    // Handle optional sign
    if (*p == '+') {
        p++;
    } else if (*p == '-') {
        sign = -1;
        p++;
    }

    // Check if there are digits
    if (!isdigit((unsigned char)*p)) {
        return -1;
    }

    // Parse digits
    while (isdigit((unsigned char)*p)) {
        val = val * 10 + (*p - '0');
        p++;
    }

    // Check for trailing characters (must be end of string)
    if (*p != '\0') {
        return -1;
    }

    // Apply sign
    val *= sign;

    // Store result
    *out = (int)val;

    return 0;
}