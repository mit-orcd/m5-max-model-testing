#include <string.h>
#include <ctype.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (cap == 0 || line == NULL || out == NULL) return -1;

    const char *p = line;
    int current_idx = 0;
    int in_quotes = 0;
    size_t out_pos = 0;

    while (*p) {
        if (in_quotes) {
            if (*p == '"' && *(p + 1) == '"') {
                if (out_pos < cap - 1) out[out_pos++] = '"';
                p += 2;
            } else if (*p == '"') {
                in_quotes = 0;
                p++;
            } else {
                if (out_pos < cap - 1) out[out_pos++] = *p;
                p++;
            }
        } else {
            if (*p == '"') {
                in_quotes = 1;
                p++;
            } else if (*p == ',') {
                current_idx++;
                p++;
                if (current_idx == idx) {
                    out[out_pos] = '\0';
                    return out_pos;
                }
                out_pos = 0;
            } else {
                if (out_pos < cap - 1) out[out_pos++] = *p;
                p++;
            }
        }
    }

    if (current_idx == idx && in_quotes) return -1;
    if (current_idx == idx) {
        out[out_pos] = '\0';
        return out_pos;
    }
    return -1;
}