int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    while (*s && count < max) {
        int sign = 1, val = 0;
        if (*s == '-') {
            sign = -1;
            s++;
        }
        while (*s >= '0' && *s <= '9') {
            val = val * 10 + (*s - '0');
            s++;
        }
        out[count++] = sign * val;
        if (*s == ',') s++;
    }
    return count;
}
