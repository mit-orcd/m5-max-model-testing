int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) return -1;
    if (*s == '\0') return -1;

    int negative = 0;
    const char *p = s;

    if (*p == '-') {
        negative = 1;
        ++p;
    } else if (*p == '+') {
        ++p;
    }

    if (*p == '\0' || *p < '0' || *p > '9') return -1;

    unsigned int value = 0;
    while (*p >= '0' && *p <= '9') {
        value = value * 10 + (unsigned int)(*p - '0');
        ++p;
    }

    if (*p != '\0') return -1;

    *out = (int)(negative ? 0u - value : value);
    return 0;
}