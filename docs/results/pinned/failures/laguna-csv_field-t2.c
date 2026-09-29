#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (idx < 0 || !line || !out || cap == 0) return -1;
    
    const char *p = line;
    int current_idx = 0;
    size_t out_pos = 0;
    
    while (current_idx <= idx && *p) {
        if (current_idx == idx) {
            if (*p == '"') {
                p++;
                while (*p) {
                    if (*p == '"') {
                        if (*(p + 1) == '"') {
                            if (out_pos < cap - 1) out[out_pos++] = '"';
                            p += 2;
                        } else if (*(p + 1) == ',' || *(p + 1) == '\0' || *(p + 1) == '\n') {
                            p++;
                            break;
                        } else {
                            if (out_pos < cap - 1) out[out_pos++] = '"';
                            p++;
                        }
                    } else {
                        if (out_pos < cap - 1) out[out_pos++] = *p;
                        p++;
                    }
                }
                if (*p) p++;
                if (out_pos < cap - 1) out[out_pos] = '\0';
                return (int)out_pos;
            } else {
                while (*p && *p != ',' && *p != '\n') {
                    if (out_pos < cap - 1) out[out_pos++] = *p;
                    p++;
                }
                if (out_pos < cap - 1) out[out_pos] = '\0';
                return (int)out_pos;
            }
        }
        
        if (*p == '"') {
            p++;
            while (*p) {
                if (*p == '"') {
                    if (*(p + 1) == '"') p += 2;
                    else break;
                } else {
                    p++;
                }
            }
            if (*p) p++;
        } else {
            while (*p && *p != ',' && *p != '\n') p++;
        }
        
        if (*p == ',') p++;
        current_idx++;
    }
    
    return -1;
}