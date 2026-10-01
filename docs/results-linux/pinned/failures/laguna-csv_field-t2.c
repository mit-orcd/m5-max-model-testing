#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (idx < 0 || !line || !out || cap == 0) return -1;
    
    const char *p = line;
    int current_idx = 0;
    char *field_start = NULL;
    size_t out_len = 0;
    
    while (*p) {
        if (current_idx == idx) {
            field_start = (char *)p;
            out_len = 0;
        }
        
        if (*p == '"') {
            p++;
            while (*p) {
                if (*p == '"') {
                    if (p[1] == '"') {
                        if (out_len < cap - 1) out[out_len] = '"';
                        out_len++;
                        p += 2;
                    } else {
                        p++;
                        break;
                    }
                } else {
                    if (out_len < cap - 1) out[out_len] = *p;
                    out_len++;
                    p++;
                }
            }
            if (current_idx == idx) {
                if (*p) p++;
            }
        } else if (*p == ',') {
            if (current_idx == idx) {
                out[out_len] = '\0';
                return (int)out_len;
            }
            current_idx++;
            p++;
        } else {
            if (current_idx == idx) {
                if (out_len < cap - 1) out[out_len] = *p;
                out_len++;
            }
            if (*p == '\r' || *p == '\n') break;
            p++;
        }
    }
    
    if (current_idx == idx) {
        out[out_len] = '\0';
        return (int)out_len;
    }
    
    return -1;
}