#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (idx < 0) return -1;
    size_t out_idx = 0;
    int field_idx = 0;
    int in_quotes = 0;
    int escaped = 0;
    const char *p = line;

    while (*p) {
        if (field_idx == idx) {
            if (out_idx < cap - 1) {
                if (escaped) {
                    out[out_idx++] = '"';
                    escaped = 0;
                } else if (in_quotes && *p == '"') {
                    if (p[1] == '"') {
                        escaped = 1;
                    } else {
                        in_quotes = 0;
                    }
                } else {
                    out[out_idx++] = *p;
                }
            } else {
                break;
            }
        }

        if (!in_quotes && *p == ',') {
            field_idx++;
            if (field_idx == idx) out_idx = 0;
        } else {
            if (*p == '"') {
                in_quotes = !in_quotes;
            }
            if (field_idx == idx && !escaped) {
                if (out_idx < cap - 1) {
                    if (!in_quotes || (p[1] != '"' && p[1] != '\0')) {
                        out[out_idx++] = *p;
                    }
                }
            }
        }
        p++;
    }

    if (field_idx < idx) return -1;
    if (out_idx < cap - 1) out[out_idx] = '\0';
    else if (cap > 0) out[cap - 1] = '\0';
    return (int)out_idx;
}