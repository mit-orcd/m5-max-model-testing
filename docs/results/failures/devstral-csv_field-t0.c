#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    const char *p = line;
    int field = 0;
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
                if (field == idx) {
                    out[out_pos] = '\0';
                    return out_pos;
                }
                field++;
                p++;
                while (*p == ' ') p++; // Skip spaces after comma
            } else {
                if (out_pos < cap - 1) out[out_pos++] = *p;
                p++;
            }
        }
    }

    if (in_quotes) return -1; // Unclosed quote

    if (field == idx) {
        out[out_pos] = '\0';
        return out_pos;
    }

    return -1;
}