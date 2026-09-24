int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || !out || cap == 0) return -1;
    *out = '\0';
    if (idx < 0) return -1;

    size_t len = 0;
    int field = 0;

    while (field < idx) {
        if (*line == '\0') return -1;
        if (*line == '"') {
            line++;
            while (*line) {
                if (*line == '"') {
                    if (line[1] == '"') {
                        line += 2;
                    } else {
                        line++;
                        break;
                    }
                } else {
                    line++;
                }
            }
            if (*line == ',') line++;
        } else {
            while (*line && *line != ',') line++;
            if (*line == ',') line++;
        }
        field++;
    }

    if (*line == '\0' && idx == 0) {
        if (cap > 0) *out = '\0';
        return 0;
    }

    int quoted = (*line == '"');
    if (quoted) line++;

    while (*line) {
        if (quoted) {
            if (*line == '"') {
                if (line[1] == '"') {
                    if (len + 1 < cap) out[len++] = '"';
                    line += 2;
                } else {
                    line++;
                    break;
                }
            } else {
                if (len + 1 < cap) out[len++] = *line;
                line++;
            }
        } else {
            if (*line == ',') break;
            if (len + 1 < cap) out[len++] = *line;
            line++;
        }
    }

    out[len] = '\0';
    return (int)len;
}