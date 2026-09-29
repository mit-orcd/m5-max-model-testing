#include <ctype.h>
#include <limits.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    if (max <= 0 || s == NULL) {
        return 0;
    }

    while (*s != '\0' && count < max) {
        int sign = 1;
        int val = 0;
        int overflow = 0;

        // Skip leading whitespace if any (though input is well-formed, robustness helps)
        while (*s == ' ' || *s == '\t') {
            s++;
        }

        if (*s == '\0') break;

        // Handle sign
        if (*s == '-') {
            sign = -1;
            s++;
        } else if (*s == '+') {
            s++;
        }

        // Parse digits
        if (!isdigit((unsigned char)*s)) {
            // If no digits found after optional sign, stop or skip? 
            // Problem says well-formed, so we assume digits follow sign if present.
            // If we hit a comma or end immediately without digits, it's malformed per spec,
            // but we'll just break to be safe.
            break;
        }

        while (isdigit((unsigned char)*s)) {
            int digit = *s - '0';
            
            // Check for overflow before multiplying
            if (val > (INT_MAX - digit) / 10) {
                overflow = 1;
                break;
            }
            val = val * 10 + digit;
            s++;
        }

        if (overflow) {
            // Handle overflow by clamping or breaking? 
            // Spec says well-formed, so overflow shouldn't happen on valid int input.
            // We'll break to avoid undefined behavior.
            break;
        }

        if (sign == -1) {
            val = -val;
        }

        out[count++] = val;

        // Skip comma if present
        if (*s == ',') {
            s++;
        }
    }

    return count;
}