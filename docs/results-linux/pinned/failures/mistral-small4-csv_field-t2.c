#include <stddef.h>
#include <ctype.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    const char *p = line;
    int field_idx = 0;
    int in_quotes = 0;
    size_t out_len = 0;

    while (*p) {
        if (in_quotes) {
            if (*p == '"') {
                if (*(p + 1) == '"') {
                    if (out_len < cap - 1) {
                        out[out_len++] = '"';
                    }
                    p += 2;
                } else {
                    in_quotes = 0;
                    p++;
                }
            } else {
                if (out_len < cap - 1) {
                    out[out_len++] = *p++;
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
                    out[out_len] = '\0';
                    return (int)out_len;
                }
                field_idx++;
                p++;
            } else {
                if (field_idx == idx) {
                    if (out_len < cap - 1) {
                        out[out_len++] = *p++;
                    } else {
                        p++;
                    }
                } else {
                    p++;
                }
            }
        }
    }

    if (field_idx == idx) {
        out[out_len] = '\0';
        return (int)out_len;
    }

    return -1;
}