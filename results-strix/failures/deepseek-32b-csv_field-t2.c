int csv_field(const char *line, int idx, char *out, size_t cap) {
    const char *start = line;
    const char *current = line;
    int field_idx = 0;
    char *out_ptr = out;
    size_t out_len = 0;
    bool in_quotes = false;
    bool escaped = false;
    
    while (*current != '\0' && *current != '\n') {
        if (in_quotes) {
            if (*current == '"') {
                if (escaped) {
                    *out_ptr++ = '"';
                    out_len++;
                    escaped = false;
                } else {
                    escaped = true;
                }
            } else {
                if (escaped) {
                    *out_ptr++ = '"';
                    out_len++;
                    escaped = false;
                }
                *out_ptr++ = *current;
                out_len++;
            }
        } else {
            if (*current == ',') {
                if (field_idx == idx) {
                    *out_ptr = '\0';
                    return out_len;
                }
                field_idx++;
                start = current + 1;
                out_ptr = out;
                out_len = 0;
            } else if (*current == '"') {
                in_quotes = true;
            } else {
                *out_ptr++ = *current;
                out_len++;
            }
        }
        current++;
    }
    
    if (field_idx == idx) {
        *out_ptr = '\0';
        return out_len;
    }
    
    if (*current == '\n') {
        if (field_idx + 1 == idx + 1) {
            *out_ptr = '\0';
            return out_len;
        }
    }
    
    return -1;
}