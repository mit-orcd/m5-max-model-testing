int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    int i = 0;
    int sign = 1;

    // Skip leading whitespace
    while (s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r' || s[i] == '\f' || s[i] == '\v') {
        i++;
    }

    // Handle sign
    if (s[i] == '+' || s[i] == '-') {
        if (s[i] == '-') {
            sign = -1;
        }
        i++;
    }

    // Check if empty after sign
    if (s[i] == '\0') {
        return -1;
    }

    // Parse digits
    int value = 0;
    while (s[i] >= '0' && s[i] <= '9') {
        value = value * 10 + (s[i] - '0');
        i++;
    }

    // Check for exactly one valid integer (no trailing chars, non-empty)
    if (s[i] != '\0') {
        return -1;
    }

    if (out != NULL) {
        *out = value * sign;
    }

    return 0;
}