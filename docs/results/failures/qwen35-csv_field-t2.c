#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    size_t len = 0;
    size_t out_idx = 0;
    int field_idx = 0;
    int in_quotes = 0;
    char c;

    if (cap == 0) return -1;

    while (1) {
        c = line[len];

        if (c == '\0') {
            if (field_idx == idx) {
                if (out_idx < cap) out[out_idx] = '\0';
                return (int)out_idx;
            }
            return -1;
        }

        if (in_quotes) {
            if (c == '"') {
                if (line[len + 1] == '"') {
                    // Unescape doubled quote
                    if (out_idx < cap - 1) {
                        out[out_idx++] = '"';
                    }
                    len += 2; // Skip both quotes
                } else {
                    // End of quoted field
                    in_quotes = 0;
                    len++; // Skip closing quote
                }
            } else {
                if (out_idx < cap - 1) {
                    out[out_idx++] = c;
                }
                len++;
            }
        } else {
            if (c == '"') {
                in_quotes = 1;
                len++;
            } else if (c == ',') {
                if (field_idx == idx) {
                    if (out_idx < cap) out[out_idx] = '\0';
                    return (int)out_idx;
                }
                field_idx++;
                out_idx = 0;
                len++;
            } else {
                if (out_idx < cap - 1) {
                    out[out_idx++] = c;
                }
                len++;
            }
        }
    }
}