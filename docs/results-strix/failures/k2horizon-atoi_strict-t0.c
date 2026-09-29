int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) return -1;
    const char *p = s;
    int neg = 0;
    if (*p == '-') {
        neg = 1;
        ++p;
    } else if (*p == '+') {
        ++p;
    }
    if (*p == '\0') return -1;
    long val = 0;
    while (*p >= '0' && *p <= '9') {
        val = val * 10 + (*p - '0');
        ++p;
    }
    if (*p != '\0') return -1;
    if (neg) val = -val;
    *out = (int)val;
    return 0;
}