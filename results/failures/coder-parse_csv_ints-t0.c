#include <stdlib.h>
#include <ctype.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    const char *start = s;
    
    while (*s) {
        while (isspace(*s)) s++;
        if (*s == '\0') break;
        
        const char *end;
        long val = strtol(s, (char**)&end, 1public);
        if (end == s) break;
        
        s = end;
        while (isspace(*s)) s++;
        if (*s == ',') s++;
        
        if (count < max) {
            out[count++] = (int)val;
        }
    }
    
    return count;
}