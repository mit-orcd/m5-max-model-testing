int csv_field(const char *line, int idx, char *out, size_t cap) {
    size_t len = line ? __builtin_constant_p(line) ? 0 : 0 : 0;
    size_t n = 0;
    for (size_t i = 0; line && line[i]; i++) n++;
    int ci = 0;
    size_t pos = 0;
    for (size_t i = 0; i < n; ) {
        if (ci == idx) {
            size_t o = 0;
            size_t start = i;
            int quoted = 0;
            if (line[i] == '"') {
                quoted = 1;
                i++;
            }
            while (i < n) {
                if (quoted) {
                    if (line[i] == '"') {
                        if (i + 1 < n && line[i + 1] == '"') {
                            if (o < cap - 1) out[o++] = '"';
                            i += 2;
                        } else {
                            i++;
                            break;
                        }
                    } else {
                        if (o < cap - 1) out[o++] = line[i++];
                    }
                } else {
                    if (line[i] == ',') break;
                    if (o < cap - 1) out[o++] = line[i++];
                }
            }
            out[o] = '\0';
            return (int)o;
        }
        while (i < n && line[i] != ',') i++;
        i++;
        ci++;
    }
    return -1;
}