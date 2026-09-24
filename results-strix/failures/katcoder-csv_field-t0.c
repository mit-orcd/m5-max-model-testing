int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (idx < 0 || !line) return -1;
    size_t len = 0;
    int in_quotes = 0;
    for (;;) {
        char c = *line;
        if (c == '\0' && !in_quotes) break;
        if (in_quotes) {
            if (c == '"') {
                if (line[1] == '"') {
                    if ((size_t)len < cap - 1) out[len] = '"';
                    len++;
                    line += 2;
                } else {
                    in_quotes = 0;
                    line++;
                }
            } else {
                if ((size_t)len < cap - 1) out[len] = c;
                len++;
                line++;
            }
        } else {
            if (c == '"') {
                in_quotes = 1;
                line++;
            } else if (c == ',') {
                if (idx == 0) {
                    if ((size_t)len < cap - 1) out[len] = '\0';
                    else out[cap - 1] = '\0';
                    return (int)len;
                }
                idx--;
                line++;
                break;
            } else {
                if ((size_t)len < cap - 1) out[len] = c;
                len++;
                line++;
            }
        }
    }
    if (idx != 0) return -1;
    if ((size_t)len < cap - 1) out[len] = '\0';
    else out[cap - 1] = '\0';
    return (int)len;
}