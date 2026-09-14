#include <stddef.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    if (s == NULL || out == NULL || max <= 0) {
        return 0;
    }

    const char *p = s;

    while (*p != '\0' && count < max) {
        int value = 0;
        int neg = 0;

        // Skip any leading whitespace? The problem says input is well-formed (digits, optional leading minus, commas).
        // So no whitespace expected, but let's be robust or strict?
        // "Well-formed" usually means we don't need to handle extra spaces.
        // Let's assume strict format: no spaces.

        // Check for optional leading minus
        if (*p == '-') {
            neg = 1;
            p++;
        }

        // Parse digits
        // Input is well-formed, so there should be at least one digit unless it's an error case,
        // but problem says well-formed.
        // However, if s is empty, loop condition handles it.
        // If we are at a comma or end, we shouldn't parse a number.
        // The loop structure: we expect a number, then a comma (if more), then next number.

        // Let's restart the logic.
        // We are at the start of a number (or end of string).

        // Actually, let's rewrite the loop carefully.

        // Reset for next iteration
        value = 0;
        neg = 0;

        if (*p == '-') {
            neg = 1;
            p++;
        }

        // Parse digits
        // Since input is well-formed, we expect digits here.
        while (*p >= '0' && *p <= '9') {
            value = value * 10 + (*p - '0');
            p++;
        }

        if (neg) {
            value = -value;
        }

        out[count++] = value;

        // Skip comma if present
        if (*p == ',') {
            p++;
        } else if (*p != '\0') {
            // Shouldn't happen in well-formed input
            break;
        }
    }

    return count;
}