int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL || *s == '\0') {
        return -1;
    }

    const char *p = s;
    int sign = 1;

    if (*p == '+' || *p == '-') {
        sign = (*p == '-') ? -1 : 1;
        p++;
        if (*p == '\0') {
            return -1;
        }
    }

    int result = 0;
    for (; *p != '\0'; p++) {
        if (*p < '0' || *p > '9') {
            return -1;
        }
        result = result * 10 + (*p - '0');
    }

    *out = sign * result;
    return 0;
}