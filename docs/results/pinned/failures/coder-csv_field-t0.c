int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || idx < 0 || !out || cap == 0) return -1;
    
    const char *p = line;
    int current_idx = 0;
    int field_len = 0;
    
    while (*p) {
        // Skip to the start of the desired field
        while (current_idx < idx) {
            if (*p == '"') {
                // Quoted field
                p++;
                while (*p) {
                    if (*p == '"') {
                        if (*(p + 1) == '"') {
                            p += 2; // Skip escaped quote
                        } else {
                            p++; // End of quoted field
                            break;
                        }
                    } else {
                        p++;
                    }
                }
            } else {
                // Unquoted field
                while (*p && *p != ',') p++;
            }
            if (*p == ',') p++;
            current_idx++;
        }
        
        // Now p points to the start of our target field
        if (current_idx != idx) return -1; // Field index out of range
        
        // Extract the field
        const char *start = p;
        int in_quotes = 0;
        int escaped = 0;
        
        if (*p == '"') {
            in_quotes = 1;
            p++;
        }
        
        const char *field_start = p;
        while (*p) {
            if (in_quotes) {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        p += 2; // Skip escaped quote
                    } else {
                        p++; // End of quoted field
                        break;
                    }
                } else {
                    p++;
                }
            } else {
                if (*p == ',') break;
                p++;
            }
        }
        
        // Copy the field to output
        size_t field_len = p - field_start;
        if (field_len >= cap) field_len = cap - 1;
        memcpy(out, field_start, field_len);
        out[field_len] = '\0';
        
        return field_len;
    }
    
    return -1; // Field index out of range
}