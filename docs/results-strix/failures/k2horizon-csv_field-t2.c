#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0)
        return -1;

    int field = 0;
    int in_quotes = 0;
    size_t out_len = 0;

    for (; *line != '\0'; line++) {
        char c = *line;

        if (in_quotes) {
            if (c == '"') {
                if (line[1] == '"') {
                    line++;
                    if (out_len + 1 < cap)
                        out[out_len++] = '"';
                } else {
                    in_quotes = 0;
                }
            } else {
                if (out_len + 1 < cap)
                    out[out_len++] = c;
            }
        } else {
            if (c == '"') {
                in_quotes = 1;
            } else if (c == ',') {
                if (field == idx)
                    break;
                field++;
            } else {
                if (out_len + 1 < cap)
                    out[out_len++] = c;
            }
        }
    }

    if (field != idx)
        return -1;

    out[out_len] = '\0';
    return (int)out_len;
}