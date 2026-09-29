int atoi_strict(const char *s, int *out) {
    const char *p = s;
    int sign = 1, val = 0;
    if (*p == '-') {
        sign = -1;
        p++;
    }
    if (!*p) return -1;   /* empty or lone "-" */
    for (; *p; p++) {
        if (*p < '0' || *p > '9') return -1;
        val = val * 10 + (*p - '0');
    }
    *out = sign * val;
    return 0;
}
