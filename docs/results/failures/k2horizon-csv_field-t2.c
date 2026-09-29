#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (cap == 0) return -1;

    const char *p = line;
    int field = 0;
    size_t len = 0;

    while (*p && field <= idx) {
        char in_quotes = 0;
        const char *start = p;

        if (*p == '"') {
            in_quotes = 1;
            p++;
            while (*p && !(in_quotes && *p == '"')) {
                if (in_quotes && *p == '"') {
                    if (p[1] == '"') {
                        if (len + 1 < cap) out[len++] = '"';
                        p += 2;
                        continue;
                    } else {
                        p++;
                        in_quotes = 0;
                        continue;
                    }
                }
                if (len + 1 < cap) out[len++] = *p;
                p++;
            }
            if (in_quotes && *p == '"') p++;
        } else {
            while (*p && *p != ',') {
                if (len + 1 < cap) out[len++] = *p;
                p++;
            }
        }

        if (field == idx) {
            out[len] = '\0';
            return (int)len;
        }

        if (*p == ',') p++;
        field++;
    }

    return -1;
}