int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (cap == 0) return -1;
    const char *p = line;
    int cur = 0;
    int in_quotes = 0;
    size_t len = 0;

    while (*p) {
        char c = *p;
        if (in_quotes) {
            if (c == '"') {
                if (p[1] == '"') {
                    if (len + 1 < cap) out[len++] = '"';
                    p += 2;
                    continue;
                } else {
                    in_quotes = 0;
                    p++;
                    continue;
                }
            } else {
                if (len + 1 < cap) out[len++] = c;
                p++;
                continue;
            }
        } else {
            if (c == '"') {
                in_quotes = 1;
                p++;
                continue;
            } else if (c == ',') {
                if (cur == idx) {
                    out[len] = '\0';
                    return (int)len;
                }
                cur++;
                p++;
                continue;
            } else {
                if (len + 1 < cap) out[len++] = c;
                p++;
                continue;
            }
        }
    }

    if (cur == idx) {
        out[len] = '\0';
        return (int)len;
    }
    return -1;
}