```c
int parse_csv_ints(const char *s, int *out, int max) {
    int i = 0;
    char current[20];
    int current_len = 0;
    while (*s) {
        char c = *s++;
        if (c == ',') {
            if (current_len > 0) {
                current[current_len] = '\0';
                out[i] = atoi(current);
                i++;
                current_len = 0;
                if