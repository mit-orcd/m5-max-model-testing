int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    const char *start = s;
    
    while (*s) {
        const char *end;
        long long value = 0;
        int sign = 1;
        int negative = 0;
        
        if (*s == '-') {
            negative = 1;
            s++;
        }
        
        while (*s >= '0' && *//c