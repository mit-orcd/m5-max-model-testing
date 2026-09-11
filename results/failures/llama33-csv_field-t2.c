#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    int field_idx = 0;
    int in_quote = 0;
    int out_idx = 0;
    int prev_quote = 0;

    for (const char *p = line; *p; p++) {
        if (in_quote) {
            if (*p == '"' && prev_quote) {
                out[out_idx++] = '"';
                prev_quote = 0;
            } else if (*p == '"') {
                prev_quote = 1;
            } else {
                out[out_idx++] = *p;
                prev_quote = 0;
            }
        } else {
            if (*p == ',') {
                if (field_idx == idx) {
                    out[out_idx] = '\0';
                    return out_idx;
                }
                field_idx++;
            } else if (*p == '"') {
                in_quote = 1;
                prev_quote = 0;
            } else {
                if (field_idx == idx) {
                    out[out_idx++] = *p;
                }
            }
        }

        if (out_idx >= cap - 1) {
            out[cap - 1] = '\0';
            return -1;
        }
    }

    if (in_quote) {
        return -1;
    }

    if (field_idx == idx) {
        out[out_idx] = '\0';
        return out_idx;
    }

    return -1;
}