#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (idx < 0 || !line || !out || cap == 0) return -1;
    
    int field_idx = 0;
    size_t out_pos = 0;
    int in_quotes = 0;
    int prev_was_quote = 0;
    
    while (*line) {
        if (field_idx == idx) {
            if (out_pos < cap - 1) {
                if (*line == '"') {
                    if (in_quotes && prev_was_quote) {
                        out[out_pos++] = '"';
                        prev_was_quote = 1;
                    } else if (in_quotes) {
                        in_quotes = 0;
                        prev_was_quote = 1;
                    } else {
                        in_quotes = 1;
                        prev_was_quote = 1;
                    }
                } else if (in_quotes) {
                    out[out_pos++] = *line;
                    prev_was_quote = 0;
                } else if (*line == ',') {
                    break;
                } else {
                    out[out_pos++] = *line;
                    prev_was_quote = 0;
                }
            } else {
                while (*line && *line != ',') {
                    if (*line == '"') {
                        if (in_quotes && prev_was_quote) {
                            prev_was_quote = 1;
                        } else if (in_quotes) {
                            in_quotes = 0;
                            prev_was_quote = 1;
                        } else {
                            in_quotes = 1;
                            prev_was_quote = 1;
                        }
                    } else {
                        in_quotes = 0;
                        prev_was_quote = 0;
                    }
                    line++;
                }
                break;
            }
        } else {
            if (*line == '"') {
                if (!in_quotes) in_quotes = 1;
            } else if (*line == ',' && !in_quotes) {
                field_idx++;
            }
        }
        line++;
    }
    
    out[out_pos] = '\0';
    
    if (field_idx != idx) return -1;
    return (int)out_pos;
}