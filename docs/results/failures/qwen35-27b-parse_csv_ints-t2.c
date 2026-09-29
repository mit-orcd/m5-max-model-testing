#include <ctype.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    if (max <= 0 || s == NULL) return 0;
    
    const char *p = s;
    
    while (count < max) {
        if (*p == '\0') break;
        
        // Skip leading whitespace (if any, though input is well-formed)
        while (*p && isspace(*p)) p++;
        
        if (*p == '\0') break;
        
        int sign = 1;
        if (*p == '-') {
            sign = -1;
            p++;
        }
        
        int val = 0;
        if (!isdigit(*p)) {
            // No digits found after optional sign
            if (*p == ',') {
                p++;
                continue;
            }
            break;
        }
        
        while (isdigit(*p)) {
            val = val * 10 + (*p - '0');
            p++;
        }
        
        out[count++] = val * sign;
        
        // Expect comma or end of string
        if (*p == ',') {
            p++;
        } else if (*p != '\0') {
            break;
        }
    }
    
    return count;
}