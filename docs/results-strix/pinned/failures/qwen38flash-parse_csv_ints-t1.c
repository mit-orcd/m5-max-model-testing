#include <stddef.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    if (s == NULL || out == NULL || max <= 0) {
        return 0;
    }

    const char *p = s;
    while (*p != '\0' && count < max) {
        // Skip leading whitespace if any (though spec says well-formed, so maybe not needed, but safe)
        // Spec says: digits, optional leading minus, commas. So no spaces expected.
        
        int negative = 0;
        if (*p == '-') {
            negative = 1;
            p++;
        }

        long long val = 0;
        int has_digit = 0;
        while (*p >= '0' && *p <= '9') {
            val = val * 10 + (*p - '0');
            has_digit = 1;
            p++;
        }

        if (!has_digit) {
            // Malformed, but spec says well-formed. Break or return current count.
            break;
        }

        if (negative) {
            val = -val;
        }

        // Clamp to int range if necessary, though spec implies well-formed ints.
        if (val > INT_MAX) {
            val = INT_MAX;
        } else if (val < INT_MIN) {
            val = INT_MIN;
        }

        out[count++] = (int)val;

        if (*p == ',') {
            p++;
        } else if (*p != '\0') {
            // Unexpected character, stop.
            break;
        }
    }

    return count;
}