#include <stddef.h>
#include <ctype.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (cap == 0) {
        *out = '\0';
        return -1;
    }

    const char *p = line;
    int field_idx = 0;
    int in_quotes = 0;
    size_t out_len = 0;

    while (*p) {
        if (in_quotes) {
            if (*p == '"') {
                if (*(p + 1) == '"') {
                    if (out_len < cap - 1) {
                        out[out_len++] = '"';
                    }
                    p += 2;
                } else {
                    in_quotes = 0;
                    p++;
                }
            } else {
                if (out_len < cap - 1) {
                    out[out_len++] = *p++;
                } else {
                    p++;
                }
            }
        } else {
            if (field_idx == idx) {
                if (*p == '"') {
                    in_quotes = 1;
                    p++;
                } else {
                    while (*p && *p != ',' && out_len < cap - 1) {
                        out[out_len++] = *p++;
                    }
                    out[out_len] = '\0';
                    return (int)out_len;
                }
            } else {
                if (*p == '"') {
                    int skip = 1;
                    while (*p && skip) {
                        p++;
                        while (*p && *p != '"') {
                            p++;
                        }
                        if (*p == '"') {
                            if (*(p + 1) == '"') {
                                p += 2;
                            } else {
                                skip = 0;
                                p++;
                            }
                        }
                    }
                    p++;
                } else {
                    while (*p && *p != ',' && out_len < cap - 1) {
                        p++;
                    }
                }
                if (*p == ',') {
                    p++;
                    field_idx++;
                }
            }
        }
    }

    if (field_idx < idx) {
        *out = '\0';
        return -1;
    }

    if (in_quotes) {
        out[out_len] = '\0';
        return -1;
    }

    out[out_len] = '\0';
    return (int)out_len;
}