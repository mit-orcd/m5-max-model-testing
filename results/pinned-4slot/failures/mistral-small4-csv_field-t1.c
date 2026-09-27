#include <stddef.h>
#include <ctype.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) return -1;

    const char *p = line;
    int field_idx = 0;
    int in_quotes = 0;
    size_t out_len = 0;

    while (*p != '\0') {
        if (in_quotes) {
            if (*p == '"') {
                if (*(p + 1) == '"') {
                    if (out_len < cap - 1) out[out_len++] = '"';
                    p += 2;
                } else {
                    in_quotes = 0;
                    p++;
                }
            } else {
                if (out_len < cap - 1) out[out_len++] = *p++;
                else p++;
            }
        } else {
            if (*p == '"') {
                in_quotes = 1;
                p++;
            } else if (*p == ',') {
                if (field_idx == idx) break;
                field_idx++;
                p++;
            } else {
                if (field_idx == idx) {
                    if (out_len < cap - 1) out[out_len++] = *p++;
                    else p++;
                } else {
                    p++;
                }
            }
        }
    }

    if (field_idx != idx) return -1;

    if (in_quotes) {
        while (*p != '\0') {
            if (*p == '"' && *(p + 1) == '"') {
                if (out_len < cap - 1) out[out_len++] = '"';
                p += 2;
            } else if (*p == '"') {
                in_quotes = 0;
                p++;
                break;
            } else {
                if (out_len < cap - 1) out[out_len++] = *p++;
                else p++;
            }
        }
    }

    if (in_quotes) return -1;

    if (out_len < cap) out[out_len] = '\0';
    else if (cap > 0) out[cap - 1] = '\0';

    return (int)out_len;
}