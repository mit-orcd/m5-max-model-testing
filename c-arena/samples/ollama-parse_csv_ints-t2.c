```c
int parse_csv_ints(const char *s, int *out, int max) {
    int count = 0;
    while (*s && count < max) {
        int neg = 0;
        if (*s == '-') {
            neg = 1;
            s++;
        }
        int val = 0;
        while (*s >= '0' && *s <= '9') {
            val = val * 10 + (*s - '0');
            s++;
        }