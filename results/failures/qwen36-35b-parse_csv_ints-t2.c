#include <ctype.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    const char *p = s;

    if (max <= 0) return 0;

    while (count < max && *p) {
        int sign = 1;
        int val = 0;
        int has_digit = 0;

        // Skip leading whitespace (optional, but robust)
        while (isspace((unsigned char)*p)) p++;

        if (*p == '-') {
            sign = -1;
            p++;
        } else if (*p == '+') {
            p++;
        }

        // Parse digits
        while (isdigit((unsigned char)*p)) {
            has_digit = 1;
            int digit = *p - '0';
            // Simple overflow check for int
            if (val > (INT32_MAX - digit) / 10) {
                // Overflow: cap or return error? 
                // For well-formed input within int range, assume valid.
                // But strictly, we should handle it. 
                // Given "well-formed", we assume it fits in int.
                // We'll just compute it; caller should ensure validity.
                // To be safe, we could break, but spec says well-formed.
            }
            val = val * 10 + digit;
            p++;
        }

        if (!has_digit) {
            // Invalid format, but spec says well-formed. 
            // If we encounter non-digit after sign, it's an error in input assumption.
            // We'll just stop or skip? Spec says well-formed, so this shouldn't happen.
            break;
        }

        out[count++] = sign * val;

        // Expect comma or end of string
        if (*p == ',') {
            p++;
        } else if (*p != '\0' && *p != '\n' && *p != '\r') {
            // Unexpected character
            break;
        }
    }

    return count;
}