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
                p++; // Skip the second quote in a quoted field
            }
        } else if (*p == ',' && !in_quotes) {
            current_idx++;
        } else if (*p == '\0') {
            return -1; // Field not found
        } else {
            p++;
        }
    }
    
    // Skip leading commas and whitespace
    while (*p == ',' || *p == ' ' || *p == '\t') {
        p++;
    }
    
    // Check if we're at the end of the line
    if (*p == '\0') return -1;
    
    // Extract the field
    size_t out_pos = 0;
    bool escaped = false;
    
    while (*p != '\0' && (in_quotes || *p != ',')) {
        if (escaped) {
            if (*p == '"') {
                if (out_pos < cap - 1) {
                    out[out_pos++] = '"';
                }
            } else {
                if (out_pos < cap - 1) {
                    out[out_pos++] = *p;
                }
            }
            escaped = false;
        } else if (*p == '"') {
            if (in_quotes) {
                // Check if this is a double quote
                if (*(p + 1) == '"') {
                    if (out_pos < cap - 1) {
                        out[out_pos++] = '"';
                    }
                    p += 2;
                    continue;
                } else {
                    // End of quoted field
                    in_quotes = false;
                }
            } else {
                // Start of quoted field
                in_quotes = true;
            }
        } else if (*p == '\\') {
            escaped = true;
        } else {
            if (out_pos < cap - 1) {
                out[out_pos++] = *p;
            }
        }
        p++;
    }
    
    out[out_pos] = '\0';
    return (int)out_pos;
}