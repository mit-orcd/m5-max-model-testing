#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (idx < 0) return -1;
    
    int current_idx = 0;
    size_t out_pos = 0;
    int in_quotes = 0;
    int field_started = 0;
    
    while (*line) {
        if (current_idx == idx && out_pos < cap - 1) {
            if (*line == '"') {
                if (in_quotes && *(line + 1) == '"') {
                    out[out_pos++] = '"';
                    line++;
                } else {
                    in_quotes = !in_quotes;
                }
            } else if (!in_quotes && *line == ',') {
                current_idx++;
                field_started = 0;
            } else {
                out[out_pos++] = *line;
                field_started = 1;
            }
        } else {
            if (!field_started && !in_quotes && *line == ',') {
                current_idx++;
                field_started = 0;
            } else if (*line == '"') {
                if (in_quotes && *(line + 1) == '"') {
                    line++;
                }
                in_quotes = !in_quotes;
            }
        }
        line++;
    }
    
    if (current_idx == idx && field_started) {
        return (int)out_pos;
    }
    return -1;
}