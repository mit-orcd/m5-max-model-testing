int atoi_strict(const char *s, int *out) {
    if (!s || !out) return -1;
    int sign = 1;
    int val = 0;
    int seen_digit = 0;

    while (*s == ' ') s++;
    if (*s == '-') {
        sign = -1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    while (*s >= '0' && *s <= '9') {
        seen_digit = 1;
        val = val * 10 + (*s - '0');
        s++;
    }

    while (*s == ' ') s++;

    if (!seen_digit || *s != '\0') return -1;

    *out = sign * val;
    return 0;
}