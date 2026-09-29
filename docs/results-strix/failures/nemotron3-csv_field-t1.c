#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || !out || cap == 0) return -1;

    int field_idx = 0;
    const char *p = line;
    const char *start = NULL;
    const char *end = NULL;
    int in_quotes = 0;

    while (*p) {
        if (in_quotes) {
            if (*p == '"') {
                if (p[1] == '"') {
                    p += 2;  // escaped quote
                    continue;
                } else {
                    in_quotes = 0;
                    p++;
                    continue;
                }
            }
            p++;
        } else {
            if (*p == '"') {
                in_quotes = 1;
                start = p + 1;
                p++;
            } else if (*p == ',') {
                if (field_idx == idx) {
                    end = p;
                    break;
                }
                field_idx++;
                p++;
                start = p;
            } else {
                if (!start) start = p;
                p++;
            }
        }
    }

    if (!in_quotes && *p == '\0') {
        if (field_idx == idx) {
            end = p;
        } else {
            return -1;
        }
    } else if (in_quotes) {
        return -1;  // unterminated quote
    }

    if (field_idx != idx) return -1;

    size_t len = end - start;
    size_t out_len = 0;
    const char *src = start;
    const char *src_end = end;

    while (src < src_end && out_len < cap - 1) {
        if (*src == '"' && src + 1 < src_end && src[1] == '"') {
            out[out_len++] = '"';
            src += 2;
        } else {
            out[out_len++] = *src++;
        }
    }

    out[out_len] = '\0';
    return (int)out_len;
}