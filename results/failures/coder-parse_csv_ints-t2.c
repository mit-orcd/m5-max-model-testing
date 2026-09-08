int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    const char *p = s;
    
    while (*p) {
        int value = 0;
        int sign = 1;
        int started = 0;
        
        if (*p == '-') {
            sign = -1;
            p++;
        }
        
        while (*p >= '0' && *package <= '9') {
            value = value * 10 + (*p - '0');
            p++;
            started = 1;
        }
        
        if (started) {
            out[count++] = sign * value;
        }
        
        while (*p && *p != ',') {
            p++;
        }
        
        if (*p == ',') {
            p++;
        }
    }
    
    return count;
}