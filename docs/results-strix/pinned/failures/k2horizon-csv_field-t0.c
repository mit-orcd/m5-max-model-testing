#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    int field = 0;
    int in_quotes = 0;
    size_t len = 0;

    while (*line) {
        char c = *line;

        if (in_quotes) {
            if (c == '"') {
                if (line[1] == '"') {
                    if (len + 1 < cap) {
                        out[len++] = '"';
                    }
                    line += 2;
                    continue;
                } else {
                    in_quotes = 0;
                    line++;
                    continue;
                }
            } else {
                if (len + 1 < cap) {
                    out[len++] = c;
                }
                line++;
                continue;
            }
        } else {
            if (c == '"') {
                in_quotes = 1;
                line++;
                continue;
            } else if (c == ',') {
                if (field == idx) {
                    break;
                }
                field++;
                line++;
                continue;
            } else {
                if (len + 1 < cap) {
                    out[len++] = c;
                }
                line++;
                continue;
            }
        }
    }

    if (field != idx) {
        return -1;
    }

    out[len] = '\0';
    return (int)len;
}