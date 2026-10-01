int atoi_strict(const char *s, int *out) {
    if (s == NULL || *s == '\0') {
        return -1;
    }

    int sign = 1;
    size_t i = 0;

    if (s[i] == '+' || s[i] == '-') {
        sign = (s[i] == '-') ? -1 : 1;
        i++;
        if (s[i] == '\0') {
            return -1;
        }
    }

    int value = 0;
    while (s[i] >= '0' && s[i] <= '9') {
        value = value * 10 + (s[i] - '0');
        i++;
    }

    if (s[i] != '\0') {
        return -1;
    }

    *out = sign * value;
    return 0;
}