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
        if (current_idx != idx) {
            // Skip to next field
            int in_quotes = 0;
            while (*p) {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        p += 2; // Skip escaped quote
                    } else {
                        in_quotes = !in_quotes;
                        p++;
                    }
                } else if (*p == ',' && !in_quotes) {
                    break;
                } else {
                    p++;
                }
            }
            if (*p == ',') p++;
            current_idx++;
            continue;
        }
        
        // Found the target field
        int in_quotes = 0;
        int field_start = 0;
        
        // Check if field starts with quote
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '"') {
            in_quotes = 1;
            p++;
            field_start = 1;
        }
        
        // Extract field content
        while (*p) {
            if (*p == '"') {
                if (*(p + 1) == '"') {
                    // Escaped quote
                    if (out_pos < cap - 1) {
                        out[out_pos++] = '"';
                    }
                    p += 2;
                } else {
                    // End of quoted field
                    if (in_quotes) {
                        in_quotes = 0;
                        p++;
                        break;
                    } else {
                        // Unexpected quote
                        if (out_pos < cap - 1) {
                            out[out_pos++] = '"';
                        }
                        p++;
                    }
                }
            } else if (*p == ',' && !in_quotes) {
                // End of field
                break;
            } else {
                if (out_pos < cap - 1) {
                    out[out_pos++] = *p;
                }
                p++;
            }
        }
        
        // Add null terminator
        out[out_pos] = '\0';
        return (int)out_pos;
    }
    
    // Field not found
    return -1;
}