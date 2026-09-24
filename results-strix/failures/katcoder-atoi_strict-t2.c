int atoi_strict(const char *s, int *out) {
    if (!s || !out) return -1;
    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r' || *s == '\f' || *s == '\v') s++;
    if (!*s) return -1;
    int neg = 0;
    if (*s == '-') { neg = 1; s++; }
    else if (*s == '+') { s++; }
    if (!*s) return -1;
    int result = 0;
    while (*s) {
        if (*s < '0' || *s > '9') return -1;
        result = result * 10 + (*s - '0');
        s++;
    }
    if (neg) result = -result;
    *out = result;
    return 0;
}