#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (idx < 0 || !line || !out || cap == 0) return -1;
    
    const char *p = line;
    int field_idx = 0;
    char *out_ptr = out;
    size_t out_remaining = cap;
    
    while (*p && field_idx <= idx) {
        if (field_idx == idx) {
            if (*p == '"') {
                p++;
                while (*p) {
                    if (*p == '"') {
                        if (*(p+1) == '"') {
                            if (out_remaining > 1) *out_ptr++ = '"';
                            out_remaining--;
                            p += 2;
                        } else {
                            p++;
                            break;
                        }
                    } else {
                        if (out_remaining > 1) *out_ptr++ = *p;
                        out_remaining--;
                        p++;
                    }
                }
            } else {
                while (*p && *p != ',') {
                    if (out_remaining > 1) *out_ptr++ = *p;
                    out_remaining--;
                    p++;
                }
            }
            if (out_remaining > 0) *out_ptr = '\0';
            else if (cap > 0) out[cap-1] = '\0';
            return out_ptr - out;
        }
        
        if (*p == '"') {
            p++;
            while (*p) {
                if (*p == '"') {
                    if (*(p+1) == '"') p += 2;
                    else { p++; break; }
                } else {
                    p++;
                }
            }
            if (*p) p++;
        } else {
            while (*p && *p != ',') p++;
            if (*p) p++;
        }
        
        field_idx++;
    }
    
    return -1;
}