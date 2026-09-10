int atoi_strict(const char *s, int *out) {
    if (!s || !out) return -1;
    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\v' || *s == '\f' || *s == '\r') ++s;
    int neg = 0;
    if (*s == '+') ++s;
    else if (*s == '-') { neg = 1; ++s; }
    if (*s < '0' || *s > '9') return -1;
    unsigned int val = 0;
    while (*s >= '0' && *s <= '9') {
        val = val * 10 + (unsigned int)(*s - '0');
        ++s;
    }
    if (*s != '\0') return -1;
    *out = neg ? (int)(-val) : (int)val;
    return 0;
}