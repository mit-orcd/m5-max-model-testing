#include <stdlib.h>
#include <string.h>

int parse_csv_ints(const char *s, int *out, int max) {
    if (s == NULL || out == NULL || max <= 0) {
        return 0;
    }

    int count = 0;
    const char *p = s;

    while (*p != '\0' && count < max) {
        // Skip leading whitespace if any (though spec says well-formed: digits, optional minus, commas)
        // But let's handle spaces just in case? Spec says "well-formed (digits, optional leading minus, commas)". 
        // So no spaces. Let's stick to strict parsing.
        
        // Parse optional sign
        int sign = 1;
        if (*p == '-') {
            sign = -1;
            p++;
        } else if (*p == '+') {
            // Spec says optional leading minus, doesn't mention plus. 
            // "well-formed (digits, optional leading minus, commas)"
            // So '+' is not part of well-formed input. We should not expect it.
            // If we encounter unexpected char, we might stop or error.
            // Since input is well-formed, we assume only '-', digits, ','.
            // If *p is not '-' and not digit, it's an error state but spec says well-formed.
            // Let's assume if it's not '-' and not digit, we stop.
        }

        // Check if we have digits
        if (*p < '0' || *p > '9') {
            // Invalid character for a number start. Stop.
            break;
        }

        long long val = 0;
        int overflow = 0;
        
        while (*p >= '0' && *p <= '9') {
            int digit = *p - '0';
            if (val > (LLONG_MAX - digit) / 10) {
                overflow = 1;
            }
            val = val * 10 + digit;
            p++;
        }

        if (overflow) {
            // Handle overflow? Spec says well-formed, so maybe no overflow.
            // But if it happens, we might clamp or stop.
            // Let's just store what we can or stop.
            // For now, let's just use the value modulo or clamp.
            // Actually, if input is well-formed integers, they should fit in int? 
            // Not necessarily, but parse_csv_ints stores into int.
            // Let's clamp or just cast.
            if (sign == -1) {
                if (val > (long long)INT_MAX + 1) {
                    val = (long long)INT_MAX + 1;
                }
            } else {
                if (val > INT_MAX) {
                    val = INT_MAX;
                }
            }
        }

        int result = (int)(sign * val);
        out[count] = result;
        count++;

        // After digits, expect comma or end of string
        if (*p == ',') {
            p++;
            // If next char is not valid start of number and not end, but spec says well-formed,
            // so after comma should be another number or end.
            // If p points to '\0' after comma, loop will end.
            // But if there's a trailing comma, *p == '\0', loop condition *p != '\0' fails? 
            // No, we check *p != '\0' at start of loop. But we already incremented p past comma.
            // If *p is '\0', loop ends. Good.
        } else if (*p == '\0') {
            break;
        } else {
            // Unexpected character. Well-formed input shouldn't have this.
            break;
        }
    }

    return count;
}