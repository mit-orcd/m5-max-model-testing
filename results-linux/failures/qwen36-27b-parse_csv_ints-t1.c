int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    while (*s && count < max) {
        while (*s == ' ') s++;
        if (*s == '\0' || *s == ',') break;
        long val = strtol(s, (char **)&s, 10);
        out[count++] = (int)val;
        while (*s && *s != ',') s++;
        if (*s == ',') s++;
    }
    return count;
}