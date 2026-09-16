int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    int i = 0;
    int num = 0;
    int neg = 0;
    while (s[i] != '\0') {
        if (s[i] == '-') {
            neg = 1;
            i++;
        }
        if (s[i] >= '0' && s[i] <= '9') {
            num = num * 10 + (s[i++] - '0');
            if (neg) {
                num = -num;
                neg = 0;
            }
        } else if (s[i] == ',') {
            out[count++] = num;
            num = 0;
            i++;
        }
    }
    if (num) {
        out[count++] = num;
    }
    return count;
}