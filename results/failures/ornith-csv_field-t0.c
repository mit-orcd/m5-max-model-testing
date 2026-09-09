int csv_field(const char *line, int idx, char *out, size_t cap) {
    size_t len = 0;
    int in_quotes = 0;

    if (idx < 0) {
        out[0] = '\0';
        return 0;
    }

    for (;;) {
        char c = *line;

        if (c == '"') {
            if (in_quotes && line[1] == '"') {
                if (len + 1 < cap) {
                    out[len] = '"';
                }
                len++;
                line += 2;
                continue;
            }
            in_quotes = !in_quotes;
            line++;
            continue;
        }

        if (c == ',') {
            if (idx == 0) {
                if (len < cap) {
                    out[len] = '\0';
                }
                return (int)len;
            }
            idx--;
            line++;
            continue;
        }

        if (c == '\0') {
            if (idx == 0) {
                if (len < cap) {
                    out[len] = '\0';
                }
                return (int)len;
            }
            out[0] = '\0';
            return -1;
        }

        if (len + 1 < cap) {
            out[len] = c;
        }
        len++;
        line++;
    }
}