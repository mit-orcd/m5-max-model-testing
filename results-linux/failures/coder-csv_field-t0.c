#include <string.h>
#include <stdbool.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || idx < 0 || !out || cap == 0) return -1;
    
    const char *p = line;
    int current_idx = 0;
    bool in_quotes = false;
    bool was_in_quotes = false;
    
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
        } else {
            p++;
        }
        if (*p == '\0') return -1;
    }
    
    // Skip leading comma if needed
    if (*p == ',') p++;
    
    // Check if we're at the start of a quoted field
    if (*p == '"') {
        in_quotes = true;
        was_in_quotes = true;
        p++;
    }
    
    // Extract the field
    size_t out_pos = 0;
    while (*p != '\0') {
        if (*p == '"') {
            if (in_quotes) {
                if (*(p + 1) == '"') {
                    // Escaped quote
                    if (out_pos < cap - 1) {
                        out[out_pos++] = '"';
                    }
                    p += 2;
                } else {
                    // End of quoted field
                    in_quotes = false;
                    p++;
                }
            } else {
                // Start of quoted field
                in_quotes = true;
                p++;
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
    
    // Null terminate
    out[out_pos] = '\0';
    
    // Return the unescaped length
    return (int)out_pos;
}