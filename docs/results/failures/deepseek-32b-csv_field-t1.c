int csv_field(const char *line, int idx, char *out, size_t cap) {
    const char *ptr = line;
    int field = 0;
    size_t out_len = 0;
    bool in_quotes = false;
    
    while (*ptr != '\0') {
        if (field == idx) {
            while (*ptr != '\0') {
                if (in_quotes) {
                    if (*ptr == '"') {
                        ptr++;
                        if (*ptr == '"') {  // handle escaped quote
                            if (out_len < cap - 1) {
                                out[out_len++] = '"';
                            }
                            ptr++;
                        } else {
                            in_quotes = false;
                        }
                    } else {
                        if (out_len < cap - 1) {
                            out[out_len++] = *ptr;
                        }
                        ptr++;
                    }
                } else {
                    if (*ptr == ',') {
                        break;
                    } else if (*ptr == '"') {
                        in_quotes = true;
                        ptr++;
                    } else {
                        if (out_len < cap - 1) {
                            out[out_len++] = *ptr;
                        }
                        ptr++;
                    }
                }
            }
            out[out_len] = '\0';
            return out_len;
        }
        
        if (in_quotes) {
            if (*ptr == '"' && ptr[1] == '"') {  // skip escaped quote
                ptr += 2;
            } else if (*ptr == '"') {
                in_quotes = false;
                ptr++;
            } else {
                ptr++;
            }
        } else {
            if (*ptr == '"') {
                in_quotes = true;
                ptr++;
            } else if (*ptr == ',') {
                field++;
                ptr++;
            } else {
                ptr++;
            }
        }
    }
    
    if (field == idx && *ptr == '\0') {
        out[0] = '\0';
        return 0;
    }
    
    return -1;
}