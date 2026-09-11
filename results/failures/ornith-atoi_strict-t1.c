int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *p = s;

    if (*p == '-') {
        p++;
    } else if (*p == '+') {
        p++;
    }

    if (*p == '\0') {
        return -1;
    }

    int result = 0;

    while (*p >= '0' && *p <= '9') {
        int digit = *p - '0';
        result = result * 10 + digit;
        p++;
    }

    if (*p != '\0') {
        return -1;
    }

    if (s[0] == '-') {
        result = -result;
    }

    *out = result;

    return 0;
}