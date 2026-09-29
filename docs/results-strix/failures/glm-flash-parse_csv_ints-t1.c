#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    const char *p = s;

    while (*p && count < max) {
        // Skip leading spaces (optional whitespace handling)
        while (*p == ' ') p++;

        // Parse the integer
        char *end;
        long val = strtol(p, &end, 10);

        // Check for valid conversion
        if (p == end) {
            // No digits found, break or return count
            break;
        }

        // Check for valid range (long to int)
        if (val < INT_MIN || val > INT_MAX) {
            // Overflow, break or return count
            break;
        }

        // Store the integer
        out[count++] = (int)val;

        // Move to next character
        p = end;

        // Skip trailing spaces
        while (*p == ' ') p++;

        // Expect a comma or end of string
        if (*p == ',') {
            p++; // consume comma
        } else if (*p != '\0') {
            // Unexpected character, break
            break;
        }
    }

    return count;
}