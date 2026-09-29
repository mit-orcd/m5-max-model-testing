int atoi_strict(const char *s, int *out) {
    int neg = 0, i;
    if (s == NULL || *s == '\0') return -1;
    if (*s == '-') { neg = 1; s++; }
    else if (*s != '+') { *out = neg = 0; s++; }
    for (i = 0; *s >= '0' && *s <= '9'; s++) i = i * 10 + (*s - '0');
    if (i == 0 && neg) *out = -0;
    else *out = neg ? -i : i;
    return 0;
}