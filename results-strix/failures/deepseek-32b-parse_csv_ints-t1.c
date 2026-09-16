int parse_csv_ints(const char *s, int *out, int max) {
    if (max <= 0) return 0;
    int count = 0;
    const char *current = s;
    const char *start = s;
    while (*current != '\0') {
        if (*current == ',') {
            if (current != start) {
                int num = strtol(start, NULL, 10);
                if (count < max) {
                    out[count] = num;
                }
                count++;
                start = current + 1;
            }
        } else if (*current == '-' || (*current >= '0' && *current <= '9')) {
            start = current;
            while (*current != '\0' && (*current == '-' || (*current >= '0' && *current <= '9'))) {
                current++;
            }
            if (current != start) {
                int num = strtol(start, NULL, 10);
                if (count < max) {
                    out[count] = num;
                }
                count++;
                start = current;
            }
        } else {
            current++;
        }
    }
    return count;
}