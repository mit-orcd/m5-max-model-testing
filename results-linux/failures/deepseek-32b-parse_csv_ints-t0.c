int parse_csv_ints(const char *s, int *out, int max) {
    int i = 0;
    int j = 0;
    int num = 0;
    int sign = 1;
    int len = strlen(s);
    
    while (i < len && j < max) {
        while (i < len && s[i] == ',') {
            i++;
        }
        if (i >= len) {
            break;
        }
        if (s[i] == '-') {
            sign = -1;
            i++;
        } else {
            sign = 1;
        }
        num = 0;
        while (i < len && isdigit(s[i])) {
            num = num * 10 + (s[i] - '0');
            i++;
        }
        out[j++] = sign * num;
    }
    
    return j;
}