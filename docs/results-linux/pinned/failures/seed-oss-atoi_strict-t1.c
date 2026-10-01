int atoi_strict(const char *s, int *out) {
    if (s == NULL || *s == '\0') {
        return -1;
    }

    int i = 0;
    int sign = 1;

    if (s[i] == '+' || s[i] == '-') {
        sign = (s[i] == '-') ? -1 : 1;
        i++;
        if (s[i] == '\0') {
            return -1;
        }
    }

    for (; s[i] != '\0'; i++) {
        if (s[i] < '0' || s[i] > '9') {
            return -1;
        }
    }

    int value = 0;
    for (int j = i; s[j] != '\0'; j++) {
        value = value * 10 + (s[j] - '0');
    }

    *out = sign * value;
    return 0;
}