int atoi_strict(const char *s, int *out) {
    if (s == NULL || *s == '\0') return -1;
    int i = 0;
    int sign = 1;
    if (s[0] == '-') {
        sign = -1;
        i = 1;
    } else if (s[0] == '+') {
        i = 1;
    }
    if (s[i] == '\0') return -1;
    int result = 0;
    while (s[i] != '\0') {
        if (s[i] < '0' || s[i] > '9') return -1;
        result = result * 10 + (s[i] - '0');
        i++;
    }
    *out = sign * result;
    return 0;
}