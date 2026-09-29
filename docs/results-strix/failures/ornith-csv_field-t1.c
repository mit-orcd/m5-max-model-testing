int csv_field(const char *line, int idx, char *out, size_t cap) {
    const char *p = line;
    int in_quotes = 0;

    for (int i = 0; i < idx; i++) {
        if (*p == '"') {
            in_quotes = 1;
            while (*p) {
                if (*p == '"') {
                    if (p[1] == '"') {
                        p += 2;
                        continue;
                    }
                    in_quotes = 0;
                    p++;
                    break;
                }
                p++;
            }
        }
        if (!*p) {
            return -1;
        }
        while (*p && *p != ',') {
            p++;
        }
        if (*p == ',') {
            p++;
        }
    }

    size_t len = 0;
    int esc = 0;

    if (*p == '"') {
        in_quotes = 1;
        p++;
        while (*p) {
            if (*p == '"') {
                if (p[1] == '"') {
                    if (len + 1 < cap) {
                        out[len] = '"';
                    }
                    len++;
                    p += 2;
                    continue;
                }
                in_quotes = 0;
                p++;
                break;
            }
            if (len + 1 < cap) {
                out[len] = *p;
            }
            len++;
            p++;
        }
        while (*p && *p != ',') {
            p++;
        }
    } else {
        while (*p && *p != ',') {
            if (len + 1 < cap) {
                out[len] = *p;
            }
            len++;
            p++;
        }
    }

    if (len < cap) {
        out[len] = '\0';
    } else {
        out[cap - 1] = '\0';
    }

    return (int)len;
}