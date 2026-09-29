#include <string.h>
#include <ctype.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || cap == 0) return -1;

    const char *p = line;
    int field_idx = 0;
    int in_quotes = 0;
    size_t out_len = 0;

    while (*p && field_idx <= idx) {
        if (in_quotes) {
            if (*p == '"' && *(p + 1) == '"') {
                if (out && out_len < cap - 1) out[out_len++] = '"';
                p += 2;
            } else if (*p == '"') {
                in_quotes = 0;
                p++;
            } else {
                if (out && out_len < cap - 1) out[out_len++] = *p;
                p++;
            }
        } else {
            if (*p == '"') {
                in_quotes = 1;
                p++;
            } else if (*p == ',') {
                field_idx++;
                p++;
                while (*p == ' ') { p++; } // Skip spaces after comma
            } else {
                p++;
            }
        }
    }

    if (field_idx != idx) return -1;

    if (out) {
        if (out_len >= cap) out_len = cap - 1;
        out[out_len] = '\0';
    }

    return (int)out_len;
}