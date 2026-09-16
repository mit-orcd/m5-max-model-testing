#include <stdio.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || idx < 0 || !out || cap == 0) return -1;
    
    const char *p = line;
    int current_idx = 0;
    int field_len = 0;
    
    while (*p) {
        // Skip to the start of the desired field
        while (*p && current_idx < idx) {
            if (*p == '"') {
                // Skip quoted field
                p++;
                while (*p && *p != '"') p++;
                if (*p == '"') p++; // Skip closing quote
            } else {
                // Skip unquoted field
                while (*p && *p != ',') p++;
            }
            if (*p == ',') p++;
            current_idx++;
        }
        
        if (current_idx != idx) return -1; // Field index out of range
        
        // Process the field
        const char *start = p;
        int in_quotes = 0;
        int escaped = 0;
        int out_pos = 0;
        
        while (*p) {
            if (escaped) {
                if (*p == '"') {
                    if (out_pos < (int)(cap - 1)) {
                        out[out_pos++] = '"';
                    }
                    escaped = 0;
                } else {
                    if (out_pos < (int)(cap - 1)) {
                        out[out_pos++] = *p;
                    }
                    escaped = 0;
                }
            } else if (*p == '"') {
                in_quotes = !in_quotes;
            } else if (*p == ',' && !in_quotes) {
                break;
            } else if (*p == '\\' && in_quotes) {
                escaped = 1;
            } else {
                if (out_pos < (int)(cap - 1)) {
                    out[out_pos++] = *p;
                }
            }
            p++;
        }
        
        out[out_pos] = '\0';
        return out_pos;
    }
    
    return -1;
}