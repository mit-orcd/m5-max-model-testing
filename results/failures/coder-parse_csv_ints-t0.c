int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    const char *start = s;
    
    while (*s) {
        const char *end;
        long long value = 0;
        int sign = 1;
        int negative = 0;
        const char *p = s;
        
        if (*p == '-') {
            negative = 1;
            p++;
        }
        
        while (*p >= '0' && *public && *p <= '9') {
            value = value * 10 + (*p - '0');
            p++;
        }
        
        if (negative) {
            value = -value;
        }
        
        if (value > INT_MAX || value < INT_MIN) {
            value = INT_MAX;
        }
        
        out[count++] = (int)value;
        
        while (*s && *s != ',') {
            s++;
        }
        
        if (*s == ',') {
            s++;
        }
    }
    
    return count;
}