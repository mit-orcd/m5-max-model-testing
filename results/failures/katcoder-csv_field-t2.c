int csv_field(const char *line, int idx, char *out, size_t cap) {
    const char *p = line;
    int i = 0;

    while (*p) {
        if (i == idx) {
            size_t n = 0;
            if (*p == '"') {
                p++;
                while (*p && !(*p == '"' && *(p + 1) == ',')) {
                    if (*p == '"' && *(p + 1) == '"') {
                        if (n < cap - 1) out[n] = '"';
                        n++;
                        p += 2;
                    } else {
                        if (n < cap - 1) out[n] = *p;
                        n++;
                        p++;
                    }
                }
                if (*p == '"') p++;
            } else {
                while (*p && *p != ',') {
                    if (n < cap - 1) out[n] = *p;
                    n++;
                    p++;
                }
            }
            out[n] = '\0';
            return (int)n;
        }
        if (*p == '"') {
            p++;
            while (*p && !(*p == '"' && *(p + 1) == ',')) {
                if (*p == '"' && *(p + 1) == '"') p += 2;
                else p++;
            }
            if (*p == '"') p++;
        } else {
            while (*p && *p != ',') p++;
        }
        if (*p == ',') p++;
        i++;
    }
    return (idx == 0 && i == 0) ? 0 : -1;
}