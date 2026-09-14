#include <string.h>
#include <ctype.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || idx < 0 || !out || cap == 0) return -1;
    
    const char *p = line;
    int current_idx = 0;
    int field_len = 0;
    
    while (*p) {
        // Skip leading whitespace
        while (isspace((unsigned char)*p)) p++;
        
        // Check if we're looking for this field
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
                        if (field_len < (int)(cap - 1)) {
                            out[field_len++] = '"';
                        }
                        p += 2;
                        continue;
                    }
                    if (field_len < (int)(cap - 1)) {
                        out[field_len++] = *p;
                    }
                } else if (*p == '"') {
                    in_quotes = !in_quotes;
                } else if (*p == ',' && !in_quotes) {
                    break;
                } else if (*p == '"' && *(p + 1) == '"') {
                    // Doubled quote
                    if (field_len < (int)(cap - 1)) {
                        out[field_len++] = '"';
                    }
                    p += 2;
                    continue;
                } else if (*p == '"' && *(p + 1) == ',') {
                    // Quote followed by comma - end of field
                    p++;
                    break;
                } else if (*p == '"' && *(p + 1) == '\0') {
                    // Quote at end of line - end of field
                    break;
                } else {
                    if (field_len < (int)(cap - 1)) {
                        out[field_len++] = *p;
                    }
                }
                p++;
            }
            
            out[field_len] = '\0';
            return field_len;
        }
        
        // Skip to next field
        while (*p && *p != ',') {
            if (*p == '"' && *(p + 1) == '"') {
                p += 2;
            } else if (*p == '"' && *(p + 1) == ',') {
                p += 2;
                break;
            } else if (*p == '"' && *(p + 1) == '\0') {
                break;
            } else {
                p++;
            }
        }
        
        // Skip comma
        if (*p == ',') p++;
        
        current_idx++;
    }
    
    return -1;
}