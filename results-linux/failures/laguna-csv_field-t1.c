#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (idx < 0) return -1;
    size_t out_idx = 0;
    int field_idx = 0;
    int in_quotes = 0;
    int escaped = 0;
    const char *p = line;

    while (*p) {
        if (field_idx == idx && out_idx < cap - 1) {
            if (escaped) {
                out[out_idx++] = '"';
                escaped = 0;
            } else if (in_quotes) {
                out[out_idx++] = *p;
            }
        }

        if (*p == '"') {
            if (in_quotes && *(p + 1) == '"') {
                if (field_idx == idx && out_idx < cap - 1) {
                    out[out_idx++] = '"';
                }
                p += 2;
                continue;
            }
            in_quotes = !in_quotes;
        } else if (*p == ',') {
            if (in_quotes) {
                if (field_idx == idx && out_idx < cap - 1) {
                    out[out_idx++] = *p;
                }
            } else {
                field_idx++;
            }
        } else {
            if (field_idx == idx && out_idx < cap - 1) {
                out[out_idx++] = *p;
            }
        }
        p++;
    }

    if (field_idx == idx && out_idx < cap - 1) {
        out[out_idx] = '\0';
        return (int)out_idx;
    }

    return -1;
}