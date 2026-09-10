#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0)
        return -1;

    int cur = 0;
    int in_quotes = 0;
    size_t out_len = 0;
    const char *p = line;

    while (*p != '\0') {
        if (in_quotes) {
            if (*p == '"') {
                if (p[1] == '"') {
                    if (out_len + 1 < cap)
                        out[out_len++] = '"';
                    p += 2;
                } else {
                    in_quotes = 0;
                    p++;
                }
            } else {
                if (out_len + 1 < cap)
                    out[out_len++] = *p;
                p++;
            }
        } else {
            if (*p == '"') {
                in_quotes = 1;
                p++;
            } else if (*p == ',') {
                if (cur == idx)
                    break;
                cur++;
                p++;
            } else {
                if (out_len + 1 < cap)
                    out[out_len++] = *p;
                p++;
            }
        }
    }

    if (cur != idx)
        return -1;

    out[out_len] = '\0';
    return (int)out_len;
}