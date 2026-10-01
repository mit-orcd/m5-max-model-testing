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
        
        // Check if we're at the right field
        if (current_idx == idx) {
            const char *start = p;
            int in_quotes = 0;
            int escaped = 0;
            
            // Parse the field
            while (*p) {
                if (escaped) {
                    escaped = 0;
                    if (*p == '"' && *(p + 1) == '"') {
                        // Doubled quote
                        if (out_pos < cap - 1) out[out_pos++] = '"';
                        p += 2;
                        continue;
                    } else {
                        // Escaped character (treat as literal)
                        if (out_pos < cap - 1) out[out_pos++] = *p;
                        p++;
                        continue;
                    }
                }
                
                if (*p == '"') {
                    if (in_quotes) {
                        // Check if this is a closing quote
                        if (*(p + 1) == '"') {
                            // Doubled quote, continue
                            p++;
                        } else {
                            // End of quoted field
                            in_quotes = 0;
                        }
                    } else {
                        // Start of quoted field
                        in_quotes = 1;
                    }
                } else if (*p == ',' && !in_quotes) {
                    // End of field
                    break;
                } else if (*p == '\\' && in_quotes) {
                    // Escape character in quoted field
                    escaped = 1;
                } else {
                    // Regular character
                    if (out_pos < cap - 1) out[out_pos++] = *p;
                }
                p++;
            }
            
            // Null terminate
            out[out_pos] = '\0';
            return (int)out_pos;
        }
        
        // Skip to next field
        int in_quotes = 0;
        while (*p) {
            if (*p == '"') {
                if (*(p + 1) == '"') {
                    p += 2;
                    continue;
                } else {
                    in_quotes = !in_quotes;
                }
            } else if (*p == ',' && !in_quotes) {
                break;
            }
            p++;
        }
        
        // Skip the comma
        if (*p == ',') p++;
        current_idx++;
    }
    
    return -1;
}