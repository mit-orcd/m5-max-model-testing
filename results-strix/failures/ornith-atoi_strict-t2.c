int atoi_strict(const char *s, int *out) {
    if (s == 0)
        return -1;

    const char *p = s;

    if (*p == '+' || *p == '-')
        p++;

    if (*p == '\0')
        return -1;

    int sign = 1;
    if (*p == '-') {
        sign = -1;
        p++;
    }

    long value = 0;
    for (; *p >= '0' && *p <= '9'; p++)
        value = value * 10 + (*p - '0');

    if (*p != '\0')
        return -1;

    *out = (int)(sign * value);
    return 0;
}