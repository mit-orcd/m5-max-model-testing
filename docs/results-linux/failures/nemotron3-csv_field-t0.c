#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || idx < 0) return -1;

    const char *p = line;
    int field_idx = 0;
    size_t out_len = 0;

    while (*p) {
        if (field_idx == idx) {
            if (*p == '"') {
                p++;
                while (*p && (out_len < cap - 1)) {
                    if (*p == '"') {
                        p++;
                        if (*p == '"') {
                            if (out_len < cap - 1) out[out_len++] = '"';
                            p++;
                        } else {
                            break;
                        }
                    } else {
                        if (out_len < cap - 1) out[out_len++] = *p;
                        p++;
                    }
                }
            } else {
                while (*p && *p != ',' && (out_len < cap - 1)) {
                    out[out_len++] = *p;
                    p++;
                }
            }
            out[out_len] = '\0';
            return (int)out_len;
        }

        if (*p == '"') {
            p++;
            while (*p && (*p != '"' || *(p+1) == '"')) {
                if (*p == '"' && *(p+1) == '"') p += 2;
                else p++;
            }
            if (*p == '"') p++;
        } else {
            while (*p && *p != ',') p++;
        }

        if (*p == ',') p++;
        field_idx++;
    }

    return -1;
}