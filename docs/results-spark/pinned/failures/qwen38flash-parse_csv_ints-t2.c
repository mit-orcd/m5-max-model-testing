#include <stdlib.h>
#include <string.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    const char *p = s;
    
    while (*p && count < max) {
        /* Skip whitespace if any (though spec says well-formed, be safe) */
        while (*p == ' ' || *p == '\t') {
            p++;
        }
        
        if (*p == '\0') {
            break;
        }
        
        /* Parse integer */
        char *endptr;
        long val = strtol(p, &endptr, 10);
        
        /* Check if we actually parsed something */
        if (endptr == p) {
            /* No digits parsed, skip to next comma or end */
            while (*p && *p != ',') {
                p++;
            }
        } else {
            /* Store the value, clamping to int range if needed */
            if (val > INT_MAX) {
                out[count] = INT_MAX;
            } else if (val < INT_MIN) {
                out[count] = INT_MIN;
            } else {
                out[count] = (int)val;
            }
            count++;
            p = endptr;
        }
        
        /* Skip comma if present */
        if (*p == ',') {
            p++;
        }
    }
    
    return count;
}