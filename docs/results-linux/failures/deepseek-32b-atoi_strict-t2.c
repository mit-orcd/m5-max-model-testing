int atoi_strict(const char *s, int *out) {
    if (s == NULL || *s == '\0') {
        return -1;
    }
    int sign = 1;
    int index = 0;
    if (s[index] == '-') {
        sign = -1;
        index++;
    }
    if (s[index] == '\0') {
        return -1;
    }
    int result = 0;
    while (s[index] != '\0') {
        if (s[index] >= '0' && s[index] <= '9') {
            result = result * 10 + (s[index] - '0');
            index++;
        } else {
            return -1;
        }
    }
    *out = sign * result;
    return 0;
}