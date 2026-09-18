int atoi_strict(const char *s, int *out) {
    if (s == NULL || *s == '\0') {
        return -1;
    }

    int sign = 1;
    if (*s == '-') {
        sign = -1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    if (*s == '\0' || !isdigit(*s)) {
        return -1;
    }

    int num = 0;
    while (*s != '\0' && isdigit(*s)) {
        num = num * 10 + (*s - '0');
        s++;
    }

    if (*s != '\0') {
        return -1;
    }

    *out = num * sign;
    return 0;
}