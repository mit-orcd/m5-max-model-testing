#include <stdio.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    const char *ptr = s;

    while (*ptr != '\0' && count < max) {
        // Skip potential leading whitespace if needed, 
        // but per prompt input is well-formed (digits, minus, commas)
        
        // Parse integer
        char *endptr;
        out[count] = (int)strtol(ptr, &endptr, 10);
        
        // If no digits were consumed, stop
        if (endptr == ptr) break;

        count++;
        ptr = endptr;

        // Skip comma if present
        if (*ptr == ',') {
            ptr++;
        }
    }

    return count;
}