int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) return -1;
    if (*s == '\0') return -1;

    int neg = 0;
    const char *p = s;
    if (*p == '-') {
        neg = 1;
        p++;
        if (*p == '\0') return -1;
    }

    int value = 0;
    int digits = 0;
    while (*p >= '0' && *p <= '9') {
        value = value * 10 + (*p - '0');
        p++;
        digits++;
    }

    if (digits == 0) return -1;
    if (*p != '\0') return -1;

    *out = neg ? -value : value;
    return 0;
}