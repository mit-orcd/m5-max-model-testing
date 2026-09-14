#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || idx < 0 || !out || cap == 0) return -1;
    
    const char *p = line;
    int current_idx = 0;
    size_t out_pos = 0;
    
    while (*p) {
        // Skip leading whitespace
        while (*p == ' ' || *p == '\t') p++;
        
        // Check if we're at the right field index
        if (current_idx == idx) {
            // Parse the field
            int is_quoted = 0;
            const char *field_start = p;
            
            // Check if field is quoted
            if (*p == '"') {
                is_quoted = 1;
                p++; // Skip opening quote
                field_start = p;
            }
            
            // Parse field content
            const char *start = p;
            while (*p) {
                if (is_quoted) {
                    // In quoted field
                    if (*p == '"') {
                        // Check for doubled quote
                        if (*(p + 1) == '"') {
                            p += 2; // Skip both quotes
                            if (out_pos < cap - 1) {
                                out[out_pos++] = '"';
                            }
                            continue;
                        } else {
                            // End of quoted field
                            p++; // Skip closing quote
                            break;
                        }
                    }
                    if (out_pos < cap - 1) {
                        out[out_pos++] = *p;
                    }
                    p++;
                } else {
                    // In unquoted field
                    if (*p == ',' || *p == '\n' || *p == '\r') {
                        break;
                    }
                    if (out_pos < cap - 1) {
                        out[out_pos++] = *p;
                    }
                    p++;
                }
            }
            
            // Null terminate
            out[out_pos] = '\0';
            
            // Return length
            return (int)(p - start);
        }
        
        // Skip to next field
        if (is_quoted) {
            // Skip quoted field
            while (*p && *p != '"') p++;
            if (*p == '"') p++; // Skip closing quote
            // Skip any doubled quotes
            while (*p == '"') p++;
        } else {
            // Skip unquoted field
            while (*p && *p != ',') p++;
        }
        
        // Skip comma
        if (*p == ',') p++;
        current_idx++;
    }
    
    return -1; // Index out of range
}