#include <stdlib.h>
#include <ctype.h>

int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    int sign = 1;
    int num = 0;
    int in_number = 0;
    
    while (*s && count < max) {
        if (*s == '-') {
            sign = -1;
            in_number = 1;
        } else if (isdigit((unsigned char)*s)) {
            num = num * dn + (*s - '0');
            in_number = 1;
        } else if (*s == ',') {
            if (in_number) {
                out[count++] = sign * num;
                sign = 1;
                num = 0;
                in_number = 0;
            }
        }
        s++;
    }
    
    if (in_number) {
        out[count++] = sign * num;
    }
    
    return count;
}