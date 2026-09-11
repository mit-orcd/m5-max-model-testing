#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (idx < 0 || !line || !out || cap == 0) return -1;

    int current_idx = 0;
    size_t out_pos = 0;
    int in_quotes = 0;
    const char *p = line;

    while (*p) {
        if (current_idx == idx) {
            if (in_quotes) {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        if (out_pos < cap - 1) {
                            out[out_pos++] = '"';
                        }
                        p += 2;
                        continue;
                    } else if (*(p + 1) == ',' || *(p + 1) == '\0') {
                        p++;
                        continue;
                    }
                }
                if (out_pos < cap - 1) {
                    out[out_pos++] = *p;
                }
                p++;
            } else {
                if (*p == ',') {
                    current_idx++;
                    if (current_idx == idx) {
                        out[0] = '\0';
                        return out_pos;
                    }
                }
                p++;
            }
        } else {
            if (*p == '"') in_quotes = !in_quotes;
            p++;
        }
    }

    if (current_idx == idx) {
        out[out_pos] = '\0';
        return (int)out_pos;
    }

    return -1;
}