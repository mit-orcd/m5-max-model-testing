#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (idx < 0) return -1;
    
    const char *p = line;
    int current_idx = 0;
    
    while (current_idx <= idx) {
        if (*p == '\0') return -1;
        
        int in_quotes = 0;
        size_t out_pos = 0;
        
        if (*p == '"') {
            in_quotes = 1;
            p++;
        }
        
        while (1) {
            if (*p == '\0' || (*p == ',' && !in_quotes)) break;
            
            if (*p == '"') {
                if (in_quotes && *(p + 1) == '"') {
                    if (out_pos < cap - 1) {
                        out[out_pos++] = '"';
                    }
                    p += 2;
                    continue;
                }
                in_quotes = 0;
                p++;
                continue;
            }
            
            if (out_pos < cap - 1) {
                out[out_pos++] = *p;
            }
            p++;
        }
        
        out[out_pos] = '\0';
        
        if (current_idx == idx) {
            return (int)out_pos;
        }
        
        if (*p == ',') p++;
        current_idx++;
    }
    
    return -1;
}