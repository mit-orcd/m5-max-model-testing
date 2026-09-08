int atoi_strict(const char *s, int *out) {
    if (!s || !out) return -1;
    
    const char *p = s;
    int negative = 0;
    
    // Skip leading whitespace
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == '\f' || *p == '\v') {
        p++;
    }
    
    // Check for sign
    if (*p == '-') {
        negative = 1;
        p++;
    } else if (*p == '+') {
        p++;
    }
    
    // Must have at least one digit
    if (*p < '0' || *p > '9') {
        return -1;
    }
    
    // Parse digits
    long long result = 0;
    while (*p >= '0' && *p <= '9') {
        result = result * 10 + (*p - '0');
        p++;
    }
    
    // No trailing characters allowed
    if (*p != '\0') {
        return -1;
    }
    
    // Apply sign
    if (negative) {
        // Check for underflow against INT_MIN
        if (result > (long long)INT_MAX + 1) {
            return -1;
        }
        *out = (int)(-result);
    } else {
        // Check for overflow against INT_MAX
        if (result > (long long)INT_MAX) {
            return -1;
        }
        *out = (int)result;
    }
    
    return 0;
}