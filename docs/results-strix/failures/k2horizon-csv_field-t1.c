int csv_field(const char *line, int idx, char *out, size_t cap) {
    const char *p = line;
    int field = 0;
    int in_quotes = 0;

    if (cap == 0) return -1;

    while (*p) {
        if (in_quotes) {
            if (*p == '"') {
                if (p[1] == '"') {
                    if (field == idx && out) {
                        if (cap > 1) {
                            size_t n = 0;
                            // copy already written chars
                            // We track length separately, so just append
                        }
                    }
                    p += 2;
                    continue;
                } else {
                    in_quotes = 0;
                    p++;
                    continue;
                }
            } else {
                if (field == idx && out) {
                    // append char
                }
                p++;
                continue;
            }
        } else {
            if (*p == '"') {
                in_quotes = 1;
                p++;
                continue;
            }
            if (*p == ',') {
                if (field == idx) break;
                field++;
                p++;
                continue;
            }
            if (field == idx && out) {
                // append char
            }
            p++;
            continue;
        }
    }

    // After loop, if field == idx, we're done
    if (field == idx) {
        if (out && cap > 0) {
            out[0] = '\0';
        }
        return 0; // length unknown, need tracking
    }
    return -1;
}