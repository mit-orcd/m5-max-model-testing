int parse_csv_ints(const char *s, int *out, int max) {
    char buffer[20];
    int buffer_idx = 0;
    int current_index = 0;
    for (int i = 0; s[i] != '\0'; ++i) {
        if (s[i] == ',') {
            buffer[buffer_idx] = '\0';
            long num = strtol(buffer, NULL, 10);
            if (current_index < max) {
                out[current_index++] = (int)num;
            }
            buffer_idx = 0;
        } else {
            if (buffer_idx < sizeof(buffer) - 1) {
                buffer[buffer_idx++] = s[i];
            }
        }
    }
    buffer[buffer_idx] = '\0';
    long num = strtol(buffer, NULL, 10);
    if (current_index < max) {
        out[current_index++] = (int)num;
    }
    return current_index;
}