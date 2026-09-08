#include <ctype.h>

int parse_csv_ints(const char *s, int *out, int max) {
    if (max <= 0 || s == NULL || out == NULL) {
        return 0;
    }

    int count = 0;
    const char *p = s;
    int current_val = 0;
    int negative = 0;
    int has_digit = 0;

    while (*p && count < max) {
        if (*p == ',') {
            if (has_digit) {
                current_val = negative ? -current_val : current_val;
                out[count++] = current_val;
                current_val = 0;
                negative = 0;
                has_digit = 0;
            }
        } else if (isdigit((unsigned char)*p)) {
            current_val = current_val * 10 + (*p - '0');
            has_digit = 1;
        } else if (*p == '-' && !has_digit && !negative && p != s) {
            // Skip potential stray minus or handle at start
            // Since input is well-formed, we assume minus is either start or part of valid number
            // If we encounter a minus after a digit, it's invalid per spec, but spec says well-formed.
            // If we encounter minus after comma, it's start of next number.
            if (count > 0 && *(p-1) == ',') {
                 negative = 1;
            }
        } else if (*p == '-' && !has_digit) {
            negative = 1;
        } else if (*p == '-' && has_digit) {
            // This case implies malformed input per spec "well-formed", but to be safe we stop or skip.
            // Given spec, we assume this won't happen.
        }

        p++;
    }

    // Handle last number if exists
    if (has_digit) {
        current_val = negative ? -current_val : current_val;
        if (count < max) {
            out[count++] = current_val;
        }
    }

    return count;
}