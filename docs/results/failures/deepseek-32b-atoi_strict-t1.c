int atoi_strict(const char *s, int *out) {
    if (s == NULL || *s == '\0') return -1;

    int sign = 1;
    int i = 0;

    if (s[i] == '-') {
        sign = -1;
        i++;
    } else if (s[i] == '+') {
        i++;
    }

    if (s[i] == '\0') return -1;

    int num = 0;
    while (s[i] != '\0' && s[i] >= '0' && s[i] <= '9') {
        num = num * 10 + (s[i] - '0');
        i++;
    }

    if (s[i] != '\0') return -1;

    *out = num * sign;
    return 0;
}