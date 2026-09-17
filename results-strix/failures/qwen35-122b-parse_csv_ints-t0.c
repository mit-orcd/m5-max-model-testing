#include <ctype.h>
#include <limits.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    if (max <= 0 || s == NULL) return 0;

    while (*s && count < max) {
        int sign = 1;
        int val = 0;
        int overflow = 0;

        // Skip leading whitespace if any (though input is well-formed)
        while (*s == ' ' || *s == '\t') s++;

        if (*s == '-') {
            sign = -1;
            s++;
        } else if (*s == '+') {
            s++;
        }

        if (!isdigit((unsigned char)*s)) {
            // If no digits found after sign, stop or skip? 
            // Assuming well-formed means if a number starts, it has digits.
            // If we hit a comma or end immediately, we stop.
            break;
        }

        while (isdigit((unsigned char)*s)) {
            int digit = *s - '0';
            if (sign == 1) {
                if (val > (INT_MAX - digit) / 10) {
                    overflow = 1;
                    break;
                }
            } else {
                if (val > (-(INT_MIN) - digit) / 10) {
                    overflow = 1;
                    break;
                }
            }
            val = val * 10 + digit;
            s++;
        }

        if (overflow) {
            // Handle overflow by clamping or stopping? 
            // Problem says well-formed, so we assume no overflow in valid inputs.
            // If it happens, we stop parsing.
            break;
        }

        out[count++] = sign * val;

        // Skip comma if present
        if (*s == ',') {
            s++;
        } else if (*s != '\0') {
            // Unexpected character, stop
            break;
        }
    }

    return count;
}