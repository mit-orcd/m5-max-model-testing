#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (idx < 0 || cap == 0) return -1;

    int current = 0;
    size_t out_pos = 0;
    int in_quotes = 0;
    int escaped = 0;

    while (*line != '\0') {
        if (!in_quotes) {
            if (*line == ',') {
                if (current == idx) {
                    out[out_pos] = '\0';
                    return (int)out_pos;
                }
                current++;
                out_pos = 0;
                line++;
                continue;
            }
            if (*line == '"') {
                in_quotes = 1;
                line++;
                continue;
            }
        } else {
            if (escaped) {
                if (out_pos < cap - 1) out[out_pos++] = *line;
                escaped = 0;
                line++;
                continue;
            }
            if (*line == '"') {
                line++;
                if (*line == '"') {
                    escaped = 1;
                    if (out_pos < cap - 1) out[out_pos++] = '"';
                    line++;
                    continue;
                }
                in_quotes = 0;
                line++;
                continue;
            }
        }

        if (current == idx) {
            if (out_pos < cap - 1) out[out_pos++] = *line;
        }
        line++;
    }

    if (current != idx) return -1;

    out[out_pos] = '\0';
    return (int)out_pos;
}