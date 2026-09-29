#include <string.h>
#include <stdbool.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || idx < 0 || !out || cap == 0) return -1;
    
    const char *p = line;
    int current_idx = 0;
    bool in_quotes = false;
    
    // Skip to the desired field
    while (current_idx < idx) {
        if (*p == '"') {
            in_quotes = !in_quotes;
            p++;
            if (in_quotes && *p == '"') {
                p++; // Skip the second quote
            }
        } else if (*p == ',' && !in_quotes) {
            current_idx++;
            p++;
        } else {
            p++;
        }
        
        if (*p == '\0') return -1;
    }
    
    // Skip leading comma if needed
    if (*p == ',') p++;
    
    // Handle quoted field
    if (*p == '"') {
        in_quotes = true;
        p++;
        size_t out_pos = 0;
        while (*p != '\0') {
            if (*p == '"') {
                if (*(p + 1) == '"') {
                    // Escaped quote
                    if (out_pos < cap - 1) {
                        out[out_pos++] = '"';
                    }
                    p += 2;
                } else {
                    // End of quoted field
                    p++;
                    break;
                }
            } else {
                if (out_pos < cap - 1) {
                    out[out_pos++] = *p;
                }
                p++;
            }
        }
        out[out_pos] = '\0';
        return out_pos;
    } else {
        // Unquoted field
        size_t out_pos = 0;
        while (*p != '\0' && *p != ',') {
            if (out_pos < cap - 1) {
                out[out_pos++] = *p;
            }
            p++;
        }
        out[out_pos] = '\0';
        return out_pos;
    }
}