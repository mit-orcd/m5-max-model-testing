int csv_field(const char *line, int idx, char *out, size_t cap) {
    int len = 0;
    int field = 0;
    int i = 0;
    int line_len = 0;
    while (line[line_len] != '\0') line_len++;

    while (i <= line_len) {
        if (field == idx) {
            int j = 0;
            int start = i;
            if (line[i] == '"') {
                i++;
                while (i < line_len) {
                    if (line[i] == '"') {
                        if (i + 1 < line_len && line[i + 1] == '"') {
                            if (j < cap - 1) out[j++] = '"';
                            i += 2;
                        } else {
                            i++;
                            break;
                        }
                    } else {
                        if (j < cap - 1) out[j++] = line[i++];
                        else i++;
                    }
                }
            } else {
                while (i < line_len && line[i] != ',') {
                    if (j < cap - 1) out[j++] = line[i++];
                    else i++;
                }
                if (i < line_len) i++;
            }
            out[j] = '\0';
            return j;
        }
        if (line[i] == '"') {
            i++;
            while (i < line_len && line[i] != '"') i++;
            if (i < line_len) i++;
            if (i < line_len && line[i] == ',') i++;
        } else {
            while (i < line_len && line[i] != ',') i++;
            if (i < line_len) i++;
        }
        field++;
    }
    return -1;
}