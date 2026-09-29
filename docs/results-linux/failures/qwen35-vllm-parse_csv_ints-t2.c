#include <ctype.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    const char *p = s;
    
    while (*p && count < max) {
        int sign = 1;
        
        // Skip leading whitespace (though input is well-formed, robustness helps)
        while (isspace((unsigned char)*p)) p++;
        
        if (*p == '-') {
            sign = -1;
            p++;
        } else if (*p == '+') {
            p++;
        }
        
        if (!isdigit((unsigned char)*p)) {
            break;
        }
        
        int val = 0;
        int overflow = 0;
        
        while (isdigit((unsigned char)*p)) {
            int digit = *p - '0';
            // Check for overflow before multiplication/addition
            if (val > (INT_MAX - digit) / 10) {
                overflow = 1;
                break;
            }
            val = val * 10 + digit;
            p++;
        }
        
        if (overflow) {
            // Handle overflow by clamping or breaking. 
            // Since input is "well-formed" as per prompt, we assume it fits int.
            // If strictly adhering to "well-formed", this path is unreachable for valid input.
            break; 
        }
        
        val *= sign;
        out[count++] = val;
        
        // Skip comma or whitespace
        while (*p == ',' || isspace((unsigned char)*p)) p++;
        
        // If we hit a non-comma non-whitespace char unexpectedly, stop
        if (*p != '\0' && !isspace((unsigned char)*p) && *p != ',') {
            break;
        }
    }
    
    return count;
}