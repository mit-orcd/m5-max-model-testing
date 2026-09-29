#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (cap == 0) return -1;
    const char *p = line;
    int field = 0;
    int in_quote = 0;
    size_t out_pos = 0;

    while (*p) {
        if (in_quote) {
            if (*p == '"') {
                if (p[1] == '"') {
                    if (out_pos < cap - 1) out[out_pos++] = '"';
                    p += 2;
                    continue;
                } else {
                    in_quote = 0;
                    p++;
                    continue;
                }
            }
            if (out_pos < cap - 1) out[out_pos++] = *p;
            p++;
        } else {
            if (*p == '"') {
                in_quote = 1;
                p++;
            } else if (*p == ',') {
                if (field == idx) {
                    out[out_pos] = '\0';
                    return (int)out_pos;
                }
                field++;
                p++;
                out_pos = 0;
            } else {
                if (out_pos < cap - 1) out[out_pos++] = *p;
                p++;
            }
        }
    }

    if (field == idx) {
        out[out_pos] = '\0';
        return (int)out_pos;
    }
    return -1;
}