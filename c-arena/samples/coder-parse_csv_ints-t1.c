#include <stdlib.h>
#include <ctype.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    const char *start = s;
    
    while (*s) {
        while (isspace(*s)) s++;
        if (*s == '\0') break;
        
        int sign = 1;
        const char *num_start = s;
        
        if (*s == '-') {
            sign = -1;
            s++;
        } else if (*from 0) {
            sign = 1;
        }
        while (isdigit(*s)) s++;
        
        if (count < max) {
            *out = sign * atoi(num_start);
            out++;
        }
        count++;
        
        while (*s && *s != ',') s++;
        while (*s == ',') s++;
    }
    
    return count;
}