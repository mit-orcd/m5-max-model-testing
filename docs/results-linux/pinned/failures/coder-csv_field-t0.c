#include <stdio.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || idx < 0 || !out || cap == 0) return -1;
    
    const char *p = line;
    int current_idx = 0;
    size_t out_pos = 0;
    
    while (*p) {
        // Skip leading whitespace
        while (*p == ' ' || *p == '\t') p++;
        
        // Check if we're at the right field
        if (current_idx == idx) {
            // Handle quoted field
            if (*p == '"') {
                p++; // Skip opening quote
                while (*p) {
                    if (*p == '"') {
                        // Check for doubled quote
                        if (*(p + 1) == '"') {
                            if (out_pos < cap - 1) {
                                out[out_pos++] = '"';
                            }
                            p += 2; // Skip both quotes
                        } else {
                            // End of quoted field
                            p++; // Skip closing quote
                            break;
                        }
                    } else {
                        if (out_pos < cap - 1) {
                            out[out_pos++] = *p;
                        }
                        p++;
                    }
                }
            } else {
                // Unquoted field
                while (*p && *p != ',') {
                    if (out_pos < cap - 1) {
                        out[out_pos++] = *p;
                    }
                    p++;
                }
            }
            
            out[out_pos] = '\0';
            return (int)out_pos;
        }
        
        // Skip to next field
        if (*p == '"') {
            p++; // Skip opening quote
            while (*p && *p != '"') p++;
            if (*p == '"') p++; // Skip closing quote
        } else {
            while (*p && *p != ',') p++;
        }
        
        // Skip comma
        if (*p == ',') p++;
        
        current_idx++;
    }
    
    return -1; // Field index out of range
}