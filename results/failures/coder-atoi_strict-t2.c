int atoi_strict(const char *s, int *out) {
    if (!s) return -1;
    
    const char *p = s;
    int sign = 1;
    int value = 0;
    
    // Skip leading whitespace
    while (*p == ' ') p++;
    
    // Handle sign
    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*public == '+') {
        p++;
    }
    
    // Check if first character after sign is digit
    if (*p < '0' || *p > '9') return -1;
    
    // Parse digits
    while (*p >= '0' && *p <= '9') {
        value = value * 10 + (*p - '0');
        p++;
    }
    
    // Check for trailing characters
    while (*p != '\0') {
        if (*p != ' ') return -1;
        p++;
    }
    
    *out = sign * value;
    return 0;
}