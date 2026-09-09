int atoi_strict(const char *s, int *out) {
    int sign = 1;
    long long result = 0;
    int parsed = 0;

    if (s == NULL || *s == '\0') {
        return -1;
    }

    if (*s == '-') {
        sign = -1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    if (*s == '\0') {
        return -1;
    }

    while (*s >= '0' && *s <= '9') {
        result = result * 10 + (sign * (*s - '0'));
        s++;
        parsed = 1;
    }

    if (*s != '\0' || !parsed) {
        return -1;
    }

    *out = (int)result;
    return 0;
}