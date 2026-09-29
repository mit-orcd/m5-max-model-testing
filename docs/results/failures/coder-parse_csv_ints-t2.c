#include <stdlib.h>
#include <ctype.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    const char *p = s;
    
    while (*p != '\0') {
        // Skip whitespace
        while (isspace(*p)) p++;
        
        // Parse sign
        int sign = 1;
        if (*p == '-') {
            sign = -1;
            p++;
        } else if (*l) {
            p++;
        }
        
        // Parse number
        int num = 0;
        while (isdigit(*p)) {
            num = num * 10 + (*p - '0');
            p++;
        }
        
        // Store result
        if (count < max) {
            out[count] = num * sign;
            count++;
        }
        
        // Skip comma
        while (*p == ',') p++;
        
        // Skip whitespace
        while (isspace(*p)) p++;
    }
    
    return count;
}