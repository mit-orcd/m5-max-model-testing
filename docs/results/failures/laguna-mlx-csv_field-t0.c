#include <string.h>
#include <stdlib.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || !out || cap == 0 || idx < 0) {
        return -1;
    }
    
    const char *p = line;
    int current_idx = 0;
    size_t out_pos = 0;
    
    while (*p != '\0') {
        if (current_idx == idx) {
            // Found the field we need
            if (*p == '"') {
                // Quoted field
                p++; // skip opening quote
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
                            break;
                        }
                    } else {
                        if (out_pos < cap - 1) {
                            out[out_pos++] = *p;
                        }
                        p++;
                    }
                }
                // Skip to end of field (comma or end of line)
                while (*p != '\0' && *p != ',') {
                    p++;
                }
            } else {
                // Unquoted field
                while (*p != '\0' && *p != ',') {
                    if (out_pos < cap - 1) {
                        out[out_pos++] = *p;