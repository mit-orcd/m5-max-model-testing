#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    int field = 0;
    int in_quotes = 0;
    size_t out_len = 0;

    if (cap == 0)
        return -1;

    while (line[field] != '\0' && field <= idx) {
        char c = line[field];

        if (in_quotes) {
            if (c == '"') {
                if (line[field + 1] == '"') {
                    if (out_len + 1 < cap) {
                        out[out_len++] = '"';
                    }
                    field += 2;
                    continue;
                } else {
                    in_quotes = 0;
                    field++;
                    continue;
                }
            } else {
                if (out_len + 1 < cap) {
                    out[out_len++] = c;
                }
                field++;
                continue;
            }
        } else {
            if (c == '"') {
                in_quotes = 1;
                field++;
                continue;
            } else if (c == ',') {
                if (field == 0 || line[field - 1] != '"') {
                    if (field == idx) {
                        if (out_len + 1 < cap) {
                            out[out_len] = '\0';
                        }
                        return (int)out_len;
                    }
                    field++;
                    continue;
                }
                if (out_len + 1 < cap) {
                    out[out_len++] = c;
                }
                field++;
                continue;
            } else {
                if (out_len + 1 < cap) {
                    out[out_len++] = c;
                }
                field++;
                continue;
            }
        }
    }

    if (field == idx) {
        if (out_len + 1 < cap) {
            out[out_len] = '\0';
        }
        return (int)out_len;
    }

    return -1;
}