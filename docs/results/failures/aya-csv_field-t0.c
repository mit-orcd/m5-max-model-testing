#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    int i, j, in_quotes, len = 0;
    const char *p = line;

    for (i = 0; i < idx; i++) {
        while (p[0] && p[0] != ',' && p[0] != '"') p++;
        if (!p[0]) return -1;
        p++;
    }

    in_quotes = p[0] == '"';
    if (in_quotes) p++;

    for (; p[0] && len < cap - 1; p++) {
        if (in_quotes && p[0] == '"') {
            if (p[1] == '"') {
                out[len++] = '"';
                p++;
            } else {
                break;
            }
        } else if (p[0] != ',' && p[0] != '\r' && p[0] != '\n') {
            out[len++] = p[0];
        }
    }

    out[len] = '\0';
    return len;
}