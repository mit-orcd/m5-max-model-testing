int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) return -1;
    const char *p = s;
    if (*p == '-') {
        p++;
    } else if (*p == '+') {
        p++;
    }
    if (*p == '\0') return -1;
    int sign = 1;
    int value = 0;
    while (*p != '\0') {
        if (*p < '0' || *p > '9') return -1;
        value = value * 10 + (*p - '0');
        p++;
    }
    if (sign < 0) value = -value;
    *out = value;
    return 0;
}