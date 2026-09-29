#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0 || idx < 0)
        return -1;

    int field = 0;
    int in_quotes = 0;
    size_t len = 0;

    for (const char *p = line; ; ++p) {
        if (*p == '\0') {
            if (field == idx) {
                if (len >= cap - 1)
                    len = cap - 1;
                out[len] = '\0';
                return (int)len;
            }
            return -1;
        }

        if (in_quotes) {
            if (*p == '"') {
                if (p[1] == '"') {
                    if (len < cap - 1)
                        out[len++] = '"';
                    ++p;
                } else {
                    in_quotes = 0;
                }
            } else {
                if (len < cap - 1)
                    out[len++] = *p;
            }
        } else {
            if (*p == '"') {
                in_quotes = 1;
            } else if (*p == ',') {
                if (field == idx) {
                    if (len >= cap - 1)
                        len = cap - 1;
                    out[len] = '\0';
                    return (int)len;
                }
                ++field;
            } else {
                if (len < cap - 1)
                    out[len++] = *p;
            }
        }
    }
}