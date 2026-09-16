int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || !out || cap == 0) return -1;
    const char *p = line;
    for (int i = 0; i < idx; i++) {
        if (*p == '"') {
            p++;
            while (*p) {
                if (*p == '"') {
                    if (*(p + 1) == '"') p += 2;
                    else { p++; break; }
                } else {
                    p++;
                }
            }
        } else {
            while (*p && *p != ',') p++;
        }
        if (*p == ',') p++;
        else return -1;
    }
    size_t len = 0;
    int quoted = (*p == '"');
    if (quoted) p++;
    while (*p) {
        if (quoted) {
            if (*p == '"') {
                if (*(p + 1) == '"') {
                    if (len < cap - 1) out[len++] = '"';
                    p += 2;
                } else {
                    p++;
                    break;
                }
            } else {
                if (len < cap - 1) out[len++] = *p++;
            }
        } else {
            if (*p == ',') break;
            if (len < cap - 1) out[len++] = *p++;
        }
    }
    out[len] = '\0';
    return (int)len;
}