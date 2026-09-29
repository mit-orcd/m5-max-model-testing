#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || idx < 0) return -1;

    const char *p = line;
    int field = 0;
    size_t out_len = 0;

    while (*p) {
        if (field == idx) {
            if (*p == '"') {
                p++;
                while (*p && *p != '"') {
                    if (*p == '"' && *(p + 1) == '"') {
                        if (out_len < cap - 1) out[out_len++] = '"';
                        p += 2;
                    } else {
                        if (out_len < cap - 1) out[out_len++] = *p;
                        p++;
                    }
                }
                if (*p == '"') p++;
                while (*p && *p != ',') p++;
                break;
            } else {
                while (*p && *p != ',') {
                    if (out_len < cap - 1) out[out_len++] = *p;
                    p++;
                }
                break;
            }
        }

        if (*p == '"') {
            p++;
            while (*p && *p != '"') {
                if (*p == '"' && *(p + 1) == '"') p += 2;
                else p++;
            }
            if (*p == '"') p++;
        } else {
            while (*p && *p != ',') p++;
        }

        if (*p == ',') p++;
        field++;
    }

    if (field != idx) return -1;

    out[out_len] = '\0';
    return (int)out_len;
}