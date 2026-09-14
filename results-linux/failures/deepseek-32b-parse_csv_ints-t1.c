int parse_csv_ints(const char *s, int *out, int max) {
    int i = 0;
    size_t start = 0;
    size_t len = strlen(s);
    while (start < len && i < max) {
        size_t end = start;
        while (end < len && s[end] != ',') {
            end++;
        }
        long num = strtol(s + start, NULL, 10);
        if (i < max) {
            out[i++] = (int)num;
        }
        start = end + 1;
    }
    return i;
}