int csv_field(const char *line, int idx, char *out, size_t cap) {
    size_t len = 0;
    const char *p = line;

    /* skip leading spaces in field */
    while (*p == ' ') p++;

    for (int i = 0; i < idx; i++) {
        /* skip current field */
        if (*p == '"') {
            p++;
            while (*p) {
                if (*p == '"') {
                    if (p[1] == '"') { p += 2; continue; }
                    p++;
                    break;
                }
                p++;
            }
        } else {
            while (*p && *p != ',') p++;
        }
        /* skip comma */
        if (*p == ',') p++;
        else return -1; /* not enough fields */
    }

    /* now parse the target field */
    if (*p == '"') {
        p++;
        while (*p) {
            if (*p == '"') {
                if (p[1] == '"') {
                    if (len + 1 >= cap) break;
                    out[len++] = '"';
                    p += 2;
                    continue;
                }
                p++;
                break;
            }
            if (len + 1 >= cap) {
                p++;
                continue;
            }
            out[len++] = *p++;
        }
    } else {
        while (*p && *p != ',') {
            if (len + 1 >= cap) { p++; continue; }
            out[len++] = *p++;
        }
    }

    out[len] = '\0';
    return (int)len;
}