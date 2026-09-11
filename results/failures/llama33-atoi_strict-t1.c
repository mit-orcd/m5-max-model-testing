int atoi_strict(const char *s, int *out) {
    while (*s && (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r')) {
        s++;
    }

    int sign = 1;
    if (*s == '-') {
        sign = -1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    int value = 0;
    while (*s && (*s >= '0' && *s <= '9')) {
        value = value * 10 + (*s - '0');
        s++;
    }

    if (s == NULL || *s != '\0' || (value == 0 && s == out)) {
        return -1;
    }

    *out = sign * value;
    return 0;
}