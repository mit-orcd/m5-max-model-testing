int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *p = s;
    int negative = 0;

    if (*p == '+') {
        p++;
    } else if (*p == '-') {
        negative = 1;
        p++;
    }

    if (*p == '\0') {
        return -1;
    }

    long result = 0;
    while (*p != '\0') {
        if (*p < '0' || *p > '9') {
            return -1;
        }
        result = result * 10 + (*p - '0');
        p++;
    }

    if (negative) {
        result = -result;
    }

    *out = (int)result;
    return 0;
}