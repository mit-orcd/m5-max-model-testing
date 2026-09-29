#include <string.h>
#include <stdlib.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    const char *p = line;
    int field_idx = 0;
    char *out_p = out;
    size_t out_len = 0;

    while (*p && field_idx <= idx) {
        if (*p == '"') {
            p++;
            while (*p && *p != '"') {
                if (*p == '"' && *(p + 1) == '"') {
                    if (out_len < cap - 1) {
                        *out_p++ = '"';
                        out_len++;
                    }
                    p += 2;
                } else {
                    if (out_len < cap - 1) {
                        *out_p++ = *p++;
                        out_len++;
                    } else {
                        p++;
                    }
                }
            }
            if (*p == '"') {
                p++;
            }
        } else {
            while (*p && *p != ',') {
                if (out_len < cap - 1) {
                    *out_p++ = *p++;
                    out_len++;
                } else {
                    p++;
                }
            }
            if (*p == ',') {
                p++;
            }
        }
        if (field_idx == idx) {
            if (out_len < cap) {
                *out_p = '\0';
            }
            return out_len;
        }
        field_idx++;
    }
    return -1;
}