#include <stdio.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    const char *p = s;

    while (*p != '\0' && count < max) {
        // Skip potential leading commas if they exist between numbers
        if (*p == ',') {
            p++;
            continue;
        }

        // Parse integer
        char *endptr;
        int val = (int)strtol(p, &endptr, 10);

        // If no digits were consumed, break or skip
        if (p == endptr) {
            // Check if it's just a trailing comma or empty
            if (*p != '\0') p++;
            continue;
        }

        out[count++] = val;
        p = endptr;

        // Skip trailing commas after a number
        while (*p == ',') {
            p++;
            // If there's nothing after the comma, we stop
            if (*p == '\0') break;
            // If it's another comma, it's an empty field (handled by loop)
            break; 
        }
    }

    // Re-implementation to handle strict comma-separated logic
    // Resetting to a cleaner approach for robust parsing
    count = 0;
    p = s;
    while (*p != '\0' && count < max) {
        char *endptr;
        long val = strtol(p, &endptr, 10);
        
        if (p == endptr) {
            // No number found, skip one character and move on
            p++;
        } else {
            out[count++] = (int)val;
            p = endptr;
            // After a number, we expect a comma or end of string
            if (*p == ',') {
                p++;
            }
        }
    }
    
    // The logic above can be simplified:
    // Since input is well-formed, we can just use strtol and skip commas.
    return 0; // Placeholder for the logic below
}