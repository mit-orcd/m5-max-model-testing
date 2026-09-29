int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || idx < 0) return -1;
    size_t len = line ? __builtin_strlen(line) : 0;
    size_t out_len = 0;
    size_t i = 0;
    while (i < len) {
        if (idx == 0) {
            if (line[i] == '"') {
                i++;
                while (i < len) {
                    if (line[i] == '"') {
                        if (i + 1 < len && line[i + 1] == '"') {
                            if (out_len < cap - 1) out[out_len++] = '"';
                            i += 2;
                        } else {
                            i++;
                            break;
                        }
                    } else {
                        if (out_len < cap - 1) out[out_len++] = line[i++];
                        else i++;
                    }
                }
            } else {
                while (i < len && line[i] != ',') {
                    if (out_len < cap - 1) out[out_len++] = line[i++];
                    else i++;
                }
            }
            break;
        }
        while (i < len && line[i] != ',') i++;
        if (i < len && line[i] == ',') i++;
        idx--;
    }
    out[out_len] = '\0';
    return out_len;
}