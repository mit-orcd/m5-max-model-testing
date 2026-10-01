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
                } else if (in_quotes) {
                    out[out_idx++] = *p;
                }
            } else {
                return -1;
            }
        }

        if (!in_quotes && *p == ',') {
            field_idx++;
            if (field_idx == idx) out_idx = 0;
            p++;
            continue;
        }

        if (in_quotes) {
            if (*p == '"') {
                if (p[1] == '"') {
                    if (field_idx == idx && out_idx < cap - 1) {
                        if (!escaped) {
                            out[out_idx++] = '"';
                        } else {
                            out[out_idx++] = '"';
                            escaped = 0;
                        }
                    }
                    p += 2;
                    continue;
                } else {
                    in_quotes = 0;
                    p++;
                    continue;
                }
            }
        } else {
            if (*p == '"') {
                in_quotes = 1;
                p++;
                continue;
            }
        }

        if (field_idx == idx && out_idx < cap - 1) {
            out[out_idx++] = *p;
        }
        p++;
    }

    if (field_idx < idx) return -1;

    if (out_idx < cap) out[out_idx] = '\0';
    else if (cap > 0) out[cap - 1] = '\0';

    return (int)out_idx;
}