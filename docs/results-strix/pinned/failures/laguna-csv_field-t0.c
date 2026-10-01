#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (idx < 0) return -1;
    size_t out_idx = 0;
    int field_idx = 0;
    int in_quotes = 0;
    int escaped = 0;
    size_t i = 0;

    while (line[i] != '\0' && field_idx <= idx) {
        char c = line[i];

        if (field_idx == idx) {
            if (out_idx < cap - 1) {
                if (escaped) {
                    out[out_idx++] = '"';
                    escaped = 0;
                } else if (in_quotes && c == '"') {
                    if (line[i+1] == '"' || line[i+1] == ',' || line[i+1] == '\0') {
                        out[out_idx++] = '"';
                        escaped = 1;
                    } else {
                        in_quotes = 0;
                    }
                } else {
                    out[out_idx++] = c;
                }
            }
        }

        if (c == '"') {
            if (!in_quotes) {
                in_quotes = 1;
            } else if (escaped) {
                escaped = 0;
            } else {
                escaped = 1;
            }
        } else {
            escaped = 0;
        }

        if (c == ',' && !in_quotes) {
            field_idx++;
            if (field_idx == idx) break;
        }

        i++;
    }

    if (out_idx < cap) out[out_idx] = '\0';
    else if (cap > 0) out[cap - 1] = '\0';

    if (field_idx < idx) return -1;

    int unescaped_len = 0;
    for (size_t j = 0; j < out_idx; j++) {
        if (out[j] != '"') unescaped_len++;
    }

    return (int)out_idx;
}