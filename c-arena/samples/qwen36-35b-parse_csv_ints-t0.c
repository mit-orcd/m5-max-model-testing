#include <ctype.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    const char *p = s;

    while (*p && count < max) {
        int sign = 1;
        // Skip whitespace (optional, but robust)
        while (isspace((unsigned char)*p)) p++;
        if (*p == '-') {
            sign = -1;
            p++;
        } else if (*p == '+') {
            p++;
        }

        if (!isdigit((unsigned char)*p)) {
            // Not a valid integer start, skip to next comma or end
            while (*p && *p != ',') p++;
            if (*p == ',') p++;
            continue;
        }

        int val = 0;
        int overflow = 0;
        while (isdigit((unsigned char)*p)) {
            int digit = *p - '0';
            // Simple overflow check for int
            if (val > (INT_MAX - digit) / 10) {
                overflow = 1;
                break;
            }
            val = val * 10 + digit;
            p++;
        }

        if (overflow) {
            // Handle overflow by clamping or skipping; here we skip
            while (*p && *p != ',') p++;
            if (*p == ',') p++;
            continue;
        }

        out[count++] = sign * val;

        // Skip to next comma
        while (*p && *p != ',') p++;
        if (*p == ',') p++;
    }

    return count;
}