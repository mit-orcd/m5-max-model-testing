int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    const char *current = s;

    while (count < max && *current != '\0') {
        const char *end = current;
        while (*end != ',' && *end != '\0') {
            end++;
        }

        int value = 0;
        int sign = 1;
        const char *num_start = current;

        if (*num_start == '-') {
            sign = -1;
            num_start++;
        }

        while (num_start < end) {
            value = value * 10 + (*num_start - '0');
            num_start++;
        }

        value *= sign;
        out[count++] = value;

        current = end + 1;
    }

    return count;
}