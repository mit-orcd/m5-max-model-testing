int atoi_strict(const char *s, int *out) {
    if (!s || !out) return -1;
    
    int sign = 1;
    int i = 0;
    
    // Skip leading whitespace
    while (s[i] == ' ' || s[i] == '\t') i++;
    
    // Check for sign
    if (s[i] == '-') {
        sign = -1;
        i++;
    } else if (s[i] == '+') {
        i++;
    }
    
    // Check if there's a digit after sign
    if (s[i] < '0' || s[i] > '9') return -1;
    
    int result = 0;
    while (s[i] >= '0' && s[i] <= '9') {
        result = result * 10 + (s[i] - '0');
        i++;
    }
    
    // Check for trailing characters
    if (s[i] != '\0') return -1;
    
    *out = result * sign;
    return 0;
}