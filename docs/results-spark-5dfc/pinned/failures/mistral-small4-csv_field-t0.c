#include <stddef.h>
#include <ctype.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    const char *p = line;
    int field_idx = 0;
    int in_quotes = 0;
    size_t out_idx = 0;
    char c;

    while (*p) {
        if (in_quotes) {
            if (*p == '"') {
                if (p[1] == '"') {
                    if (out_idx < cap - 1) {
                        out[out_idx++] = '"';
                    }
                    p += 2;
                } else {
                    in_quotes = 0;
                    p++;
                }
            } else {
                if (out_idx < cap - 1) {
                    out[out_idx++] = *p++;
                } else {
                    p++;
                }
            }
        } else {
            if (*p == '"') {
                in_quotes = 1;
                p++;
            } else if (*p == ',') {
                if (field_idx == idx) {
                    if (out_idx < cap) {
                        out[out_idx] = '\0';
                    } else {
                        out[cap - 1] = '\0';
                    }
                    return (int)out_idx;
                }
                field_idx++;
                p++;
                out_idx = 0;
            } else {
                if (field_idx == idx) {
                    if (out_idx < cap - 1) {
                        out[out_idx++] = *p;
                    }
                    p++;
                } else {
                    p++;
                }
            }
        }

        if (*p == '\0' && field_idx == idx) {
            if (out_idx < cap) {
                out[out_idx] = '\0';
            } else {
                out[cap - 1] = '\0';
            }
            return (int)out_idx;
        }
    }

    return -1;
}