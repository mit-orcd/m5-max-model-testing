#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) {
        return -1;
    }

    const char *p = line;
    int count = 0;
    size_t out_len = 0;

    while (*p) {
        if (count == idx) {
            break;
        }

        if (*p == '"') {
            p++;
            while (*p) {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        p += 2;
                        if (out_len < cap - 1) {
                            out[out_len++] = '"';
                        }
                    } else {
                        p++;
                        count++;
                        break;
                    }
                } else {
                    if (out_len < cap - 1) {
                        out[out_len++] = *p;
                    }
                    p++;
                }
            }
        } else {
            while (*p && *p != ',') {
                p++;
            }
            count++;
            if (*p == ',') {
                p++;
            }
        }
    }

    if (count != idx) {
        return -1;
    }

    out_len = 0;
    if (*p == '"') {
        p++;
        while (*p) {
            if (*p == '"') {
                if (*(p + 1) == '"') {
                    p += 2;
                    if (out_len < cap - 1) {
                        out[out_len++] = '"';
                    }
                } else {
                    p++;
                    break;
                }
            } else {
                if (out_len < cap - 1) {
                    out[out_len++] = *p;
                }
                p++;
            }
        }
    } else {
        while (*p && *p != ',' && *p != '\n' && *p != '\r') {
            if (out_len < cap - 1) {
                out[out_len++] = *p;
            }
            p++;
        }
    }

    out[out_len] = '\0';
    return (int)out_len;
}