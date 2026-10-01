int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    const char *current = s;
    while (count < max && *current != '\0') {
        const char *end = current;
        while (*end != ',' && *end != '\0') {
            end++;
        }
        int num;
        sscanf(current, "%d", &num);
        out[count++] = num;
        if (*end == '\0') {
            break;
        }
        current = end + 1;
    }
    return count;
}