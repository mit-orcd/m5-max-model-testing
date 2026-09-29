int parse_csv_ints(const char *s, int *out, int max) {
    if (!s || !out || max <= 0) {
        return 0;
    }

    int count = 0;
    int sign = 1;
    long value = 0;
    int in_number = 0;

    while (*s) {
        char c = *s;

        if (c >= '0' && c <= '9') {
            value = value * 10 + (c - '0');
            in_number = 1;
        } else if (c == '-' && !in_number) {
            sign = -1;
        } else if (c == ',' && in_number) {
            if (count < max) {
                out[count] = (int)(sign * value);
            }
            count++;
            value = 0;
            sign = 1;
            in_number = 0;
        }

        s++;
    }

    if (in_number) {
        if (count < max) {
            out[count] = (int)(sign * value);
        }
        count++;
    }

    return count;
}