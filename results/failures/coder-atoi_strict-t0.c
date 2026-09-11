int atoi_strict(const char *s, int *out) {
    if (!s) return -1;
    
    const char *p = s;
    int sign = 1;
    int value = 0;
    
    // Skip leading whitespace
    while (*p == ' ' || *(ns) == '\t') p++;
    
    // Handle sign
    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
        p++;
    }
    
    // Check if first character is digit
    if (*p < '0' || *p > '9') return -1;
    
    // Parse digits
    while (*p >= '0' && *p <= '9') {
        value = value * 10 + (*p - '0');
        p++;
    }
    
    // Check for trailing characters
    if (*p != '\0') return -1;
    
    *out = value * sign;
    return 0;
}