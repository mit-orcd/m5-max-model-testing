int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (idx < 0 || !line || !out || cap == 0) return -1;
    
    const char *p = line;
    int current_idx = 0;
    int in_quotes = 0;
    
    // Skip to the desired field
    while (current_idx < idx) {
        if (*p == '"') {
            in_quotes = !in_quotes;
            p++;
            continue;
        }
        if (*p == ',' && !in_quotes) {
            current_idx++;
            p++;
            continue;
        }
        if (*p == '\0') return -1;
        p++;
    }
    
    // Skip leading comma if needed
    if (*p == ',') p++;
    
    // Parse the field
    char *out_start = out;
    size_t out_len = 0;
    
    while (*p != '\0') {
        if (*p == '"') {
            if (in_quotes) {
                // Check for doubled quote
                if (*(p + 1) == '"') {
                    if (out_len < cap - 1) {
                        *out++ = '"';
                        out_len++;
                    }
                    p += 2;
                    continue;
                } else {
                    // End of quoted field
                    in_quotes = 0;
                    p++;
                    continue;
                }
            } else {
                in_quotes = 1;
                p++;
                continue;
            }
        }
        
        if (*p == ',' && !in_quotes) {
            break;
        }
        
        if (out_len < cap - 1) {
            *out++ = *p;
            out_len++;
        }
        p++;
    }
    
    *out = '\0';
    return (int)out_len;
}