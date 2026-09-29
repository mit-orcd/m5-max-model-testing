#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    const char *p = line;
    int current = 0;
    size_t out_len = 0;

    while (*p && current <= idx) {
        if (*p == '"') {
            if (current == idx) {
                p++;
                while (*p && out_len < cap - 1) {
                    if (*p == '"') {
                        if (*(p + 1) == '"') {
                            out[out_len++] = '"';
                            p += 2;
                        } else {
                            p++;
                            break;
                        }
                    } else {
                        out[out_len++] = *p++;
                    }
                }
                out[out_len] = '\0';
                if (*p == ',') p++;
                return (int)out_len;
            } else {
                p++;
                while (*p) {
                    if (*p == '"') {
                        if (*(p + 1) == '"') {
                            p += 2;
                        } else {
                            p++;
                            break;
                        }
                    } else {
                        p++;
                    }
                }
                if (*p == ',') p++;
                current++;
            }
        } else {
            if (current == idx) {
                while (*p && *p != ',' && out_len < cap - 1) {
                    out[out_len++] = *p++;
                }
                out[out_len] = '\0';
                if (*p == ',') p++;
                return (int)out_len;
            } else {
                while (*p && *p != ',') p++;
                if (*p == ',') p++;
                current++;
            }
        }
    }

    return -1;
}