#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || idx < 0) return -1;

    int field_idx = 0;
    const char *p = line;
    int in_quotes = 0;
    size_t out_len = 0;

    while (*p) {
        if (in_quotes) {
            if (*p == '"') {
                if (p[1] == '"') {
                    p++; // skip escaped quote
                    if (out_len < cap - 1) out[out_len++] = '"';
                } else {
                    in_quotes = 0;
                }
            } else {
                if (out_len < cap - 1) out[out_len++] = *p;
            }
        } else {
            if (*p == ',') {
                if (field_idx == idx) break;
                field_idx++;
            } else if (*p == '"') {
                in_quotes = 1;
            } else {
                if (field_idx == idx) {
                    if (out_len < cap - 1) out[out_len++] = *p;
                }
            }
        }
        p++;
    }

    if (field_idx != idx) return -1;

    out[out_len] = '\0';
    return (int)out_len;
}