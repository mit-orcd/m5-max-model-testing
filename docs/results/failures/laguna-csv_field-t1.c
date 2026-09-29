#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || !out || cap == 0) return -1;
    
    int field = 0;
    const char *p = line;
    size_t out_len = 0;
    
    while (*p) {
        if (*p == '"') {
            // Quoted field
            if (field == idx) {
                p++;
                while (*p) {
                    if (*p == '"') {
                        if (p[1] == '"') {
                            // Escaped quote
                            if (out_len < cap - 1) out[out_len++] = '"';
                            p += 2;
                        } else {
                            // End of quoted field
                            break;
                        }
                    } else {
                        if (out_len < cap - 1) out[out_len++] = *p;
                        p++;
                    }
                }
                out[out_len] = '\0';
                return (int)out_len;
            }
            // Skip to end of quoted field
            p++;
            while (*p) {
                if (*p == '"') {
                    if (p[1] == '"') {
                        p += 2;
                    } else {
                        p++;
                        // Skip to comma or end
                        while (*p && *p != ',') p++;
                        break;
                    }
                } else {
                    p++;
                }
            }
        } else {
            // Unquoted field
            if (field == idx) {
                while (*p && *p != ',') {
                    if (out_len < cap - 1) out[out_len++] = *p;
                    p++;
                }
                out[out_len] = '\0';
                return (int)out_len;
            }
        }
        
        // Move to next field
        if (*p == ',') p++;
        field++;
    }
    
    return -1;
}