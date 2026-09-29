#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || !out || cap == 0) return -1;
    
    const char *p = line;
    int current_idx = 0;
    size_t out_len = 0;
    int in_quotes = 0;

    while (*p) {
        if (current_idx == idx) {
            // Start extracting
            while (*p) {
                if (in_quotes) {
                    if (*p == '"') {
                        if (*(p + 1) == '"') {
                            // Escaped quote
                            if (out_len < cap - 1) {
                                out[out_len++] = '"';
                            }
                            p += 2;
                        } else {
                            // End of quoted field
                            in_quotes = 0;
                            p++;
                            break;
                        }
                    } else {
                        if (out_len < cap - 1) {
                            out[out_len++] = *p;
                        }
                        p++;
                    }
                } else {
                    if (*p == '"') {
                        in_quotes = 1;
                        p++;
                    } else if (*p == ',') {
                        // End of field
                        break;
                    } else {
                        if (out_len < cap - 1) {
                            out[out_len++] = *p;
                        }
                        p++;
                    }
                }
            }
            out[out_len] = '\0';
            return (int)out_len;
        }

        // Skip to next field
        while (*p) {
            if (in_quotes) {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        p += 2;
                    } else {
                        in_quotes = 0;
                        p++;
                    }
                } else {
                    p++;
                }
            } else {
                if (*p == '"') {
                    in_quotes = 1;
                    p++;
                } else if (*p == ',') {
                    p++;
                    break;
                } else {
                    p++;
                }
            }
        }
        
        if (*p == '\0') {
            return -1; // idx out of range
        }
        current_idx++;
    }

    return -1; // idx out of range
}