int csv_field(const char *line, int idx, char *out, size_t cap) {
    const char *p = line;
    int i = 0;
    size_t n = 0;

    while (i < idx) {
        if (*p == '"') {
            p++;
            while (*p) {
                if (*p == '"') {
                    if (p[1] == '"') {
                        p += 2;
                    } else {
                        p++;
                        break;
                    }
                } else {
                    p++;
                }
            }
        } else {
            while (*p && *p != ',') {
                p++;
            }
        }
        if (*p == ',') {
            p++;
        }
        i++;
    }

    if (*p == '"') {
        p++;
        while (*p) {
            if (n + 1 < cap) {
                if (*p == '"') {
                    if (p[1] == '"') {
                        out[n++] = '"';
                        p += 2;
                        continue;
                    } else {
                        p++;
                        break;
                    }
                } else {
                    out[n++] = *p++;
                }
            } else {
                if (*p == '"') {
                    if (p[1] == '"') {
                        p += 2;
                        continue;
                    }
                }
                p++;
            }
        }
    } else {
        while (*p && *p != ',') {
            if (n + 1 < cap) {
                out[n++] = *p;
            }
            p++;
        }
    }

    out[n] = '\0';
    return (int)n;
}