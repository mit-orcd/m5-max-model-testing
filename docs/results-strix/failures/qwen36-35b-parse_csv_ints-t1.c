int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    const char *p = s;
    int sign = 1;
    int val = 0;
    int in_number = 0;

    while (*p) {
        if (*p == ',' || *p == '\0') {
            if (in_number) {
                if (count < max) {
                    out[count] = sign * val;
                }
                count++;
                in_number = 0;
                val = 0;
                sign = 1;
            }
        } else if (*p == '-') {
            if (in_number) {
                if (count < max) {
                    out[count] = sign * val;
                }
                count++;
                in_number = 0;
                val = 0;
                sign = 1;
            }
            sign = -1;
            in_number = 1;
        } else if (*p >= '0' && *p <= '9') {
            in_number = 1;
            val = val * 10 + (*p - '0');
        }
        p++;
    }

    return count;
}