int atoi_strict(const char *s, int *out) {
    if (!s) return -1;
    
    const char *p = s;
    int sign = 1;
    int value = 0;
    
    // Skip leading whitespace
    while (*p == ' ' || * halted by whitespace or non-digit
        if (*p < '0' || *p > '9') return -1;
        value = value * 10 + (*p - '0');
        p++;
    }
    
    // Check for trailing characters
    if (*p != '\0') return -1;
    
    *out = sign * value;
    return 0;
}