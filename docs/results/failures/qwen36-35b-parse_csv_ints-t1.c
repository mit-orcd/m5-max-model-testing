#include <ctype.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    const char *p = s;

    if (s == NULL || out == NULL || max <= 0) {
        return 0;
    }

    while (*p && count < max) {
        int neg = 0;
        int val = 0;

        // Skip leading whitespace (though input is well-formed, be robust)
        while (*p == ' ' || *p == '\t') {
            p++;
        }

        if (*p == '-') {
            neg = 1;
            p++;
        } else if (*p == '+') {
            p++;
        }

        if (!isdigit((unsigned char)*p)) {
            break;
        }

        while (isdigit((unsigned char)*p)) {
            val = val * 10 + (*p - '0');
            p++;
        }

        out[count++] = neg ? -val : val;

        // Skip optional whitespace after number
        while (*p == ' ' || *p == '\t') {
            p++;
        }

        // Expect comma or end of string
        if (*p == ',') {
            p++;
            // Skip whitespace after comma
            while (*p == ' ' || *p == '\t') {
                p++;
            }
        } else if (*p != '\0') {
            // Malformed or trailing non-comma, non-null
            break;
        }
    }

    return count;
}