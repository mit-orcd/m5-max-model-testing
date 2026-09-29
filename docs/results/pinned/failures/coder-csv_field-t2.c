int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || idx < 0 || !out || cap == 0) return -1;
    
    const char *p = line;
    int current_idx = 0;
    int field_len = 0;
    
    while (*p) {
        // Skip to the start of the desired field
        while (current_idx < idx) {
            // Skip to next comma or end of line
            while (*p && *p != ',') {
                if (*p == '"' && *(p+1) == '"') p += 2; // Skip escaped quote
                else if (*p == '"') {
                    // Skip quoted field
                    p++;
                    while (*p && *p != '"') {
                        if (*p == '"' && *(p+1) == '"') p += 2;
                        else p++;
                    }
                    if (*p == '"') p++;
                } else p++;
            }
            if (*p == ',') p++;
            current_idx++;
        }
        
        // Now we're at the start of the desired field
        if (current_idx != idx) return -1; // Field index out of range
        
        // Handle quoted field
        if (*p == '"') {
            p++; // Skip opening quote
            while (*p) {
                if (*p == '"') {
                    if (*(p+1) == '"') {
                        // Escaped quote
                        if (field_len < (int)(cap - 1)) {
                            out[field_len++] = '"';
                        }
                        p += 2;
                    } else {
                        // Closing quote
                        p++;
                        break;
                    }
                } else {
                    if (field_len < (int)(cap - 1)) {
                        out[field_len++] = *p;
                    }
                    p++;
                }
            }
        } else {
            // Unquoted field
            while (*p && *p != ',') {
                if (field_len < (int)(cap - 1)) {
                    out[field_len++] = *p;
                }
                p++;
            }
        }
        
        // Terminate the output string
        out[field_len] = '\0';
        return field_len;
    }
    
    // If we reach here, the field index was out of range
    return -1;
}