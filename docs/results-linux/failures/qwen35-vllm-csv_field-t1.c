#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || !out) return -1;
    
    size_t len = 0;
    int field_idx = 0;
    int in_quotes = 0;
    const char *p = line;
    
    while (1) {
        char c = *p;
        
        if (c == '"') {
            if (in_quotes) {
                if (*(p + 1) == '"') {
                    // Escaped quote
                    if (len + 1 < cap) {
                        out[len++] = '"';
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
                if (len == 0 && (p == line || *(p - 1) == ',')) {
                    in_quotes = 1;
                    p++;
                    continue;
                }
            }
        }
        
        if (c == ',' || c == '\0' || c == '\n' || c == '\r') {
            if (field_idx == idx) {
                out[len] = '\0';
                return (int)len;
            }
            if (c == '\0') return -1;
            if (c == '\n' || c == '\r') {
                // Handle CRLF at end of line
                if (c == '\r' && *(p+1) == '\n') {
                    p++;
                }
                return -1;
            }
            field_idx++;
            p++;
        } else {
            if (len + 1 < cap) {
                out[len++] = c;
            }
            p++;
        }
    }
}