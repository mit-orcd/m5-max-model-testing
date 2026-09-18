```c
int parse_csv_ints(const char *s, int *out, int max) {
    const char *current = s;
    int i = 0;
    int count = 0;

    while (current != NULL && i < max) {
        char *end;
        long num = strtol((char *)current, &end, 10);
        if (end == (char *)current) {
            break;
        }
        out[i] = (int)num;
        i++;
        count++;
        if (*end == ',') {
            current = end