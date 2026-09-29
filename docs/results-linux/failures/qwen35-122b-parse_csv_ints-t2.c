#include <ctype.h>

int parse_csv_ints(const char *s, int *out, int max) {
    if (s == NULL || out == NULL || max <= 0) {
        return 0;
    }

    int count = 0;
    const char *p = s;

    while (*p != '\0' && count < max) {
        int is_negative = 0;
        int value = 0;
        int overflow_safe = 1;

        // Skip leading whitespace (optional, though spec says well-formed)
        while (*p == ' ' || *p == '\t') p++;

        if (*p == '-') {
            is_negative = 1;
            p++;
        } else if (*p == '+') {
            p++;
        }

        if (!isdigit((unsigned char)*p)) {
            break;
        }

        while (isdigit((unsigned char)*p)) {
            int digit = *p - '0';
            // Simple overflow check for positive accumulation before sign application
            if (value > 214748364 || (value == 214748364 && digit > (is_negative ? 7 : 8))) {
                overflow_safe = 0;
                break;
            }
            value = value * 10 + digit;
            p++;
        }

        if (!overflow_safe) {
            // Handle overflow by saturating or breaking; here we break to stop parsing
            break;
        }

        if (is_negative) {
            value = -value;
        }

        out[count++] = value;

        // Skip comma if present
        if (*p == ',') {
            p++;
            // Skip whitespace after comma
            while (*p == ' ' || *p == '\t') p++;
        } else if (*p != '\0') {
            // Unexpected character, stop
            break;
        }
    }

    return count;
}