#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    int field_idx = 0;
    int in_quote = 0;
    int out_idx = 0;
    int prev_quote = 0;

    while (*line) {
        if (in_quote) {
            if (*line == '"' && prev_quote) {
                prev_quote = 0;
                if (out_idx < cap - 1) {
                    out[out_idx++] = '"';
                }
            } else if (*line == '"') {
                prev_quote = 1;
            } else {
                prev_quote = 0;
                if (out_idx < cap - 1) {
                    out[out_idx++] = *line;
                }
            }
        } else {
            if (*line == ',') {
                field_idx++;
                if (field_idx == idx) {
                    out[out_idx] = '\0';
                    return out_idx;
                }
                out_idx = 0;
            } else if (*line == '"') {
                in_quote = 1;
            } else if (field_idx == idx) {
                if (out_idx < cap - 1) {
                    out[out_idx++] = *line;
                }
            }
        }
        line++;
    }

    if (field_idx == idx) {
        out[out_idx] = '\0';
        return out_idx;
    }

    return -1;
}