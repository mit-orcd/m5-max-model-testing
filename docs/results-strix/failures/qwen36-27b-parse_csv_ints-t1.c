int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    if (s == NULL || out == NULL || max <= 0) {
        return 0;
    }

    while (*s && count < max) {
        int sign = 1;
        long num = 0;

        if (*s == '-') {
            sign = -1;
            s++;
        }

        while (*s >= '0' && *s <= '9') {
            num = num * 10 + (*s - '0');
            s++;
        }

        out[count++] = (int)(sign * num);

        if (*s == ',') {
            s++;
        }
    }

    return count;
}