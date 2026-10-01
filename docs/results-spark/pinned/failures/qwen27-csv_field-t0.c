#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (cap == 0) return -1;

    const char *p = line;
    int field_idx = 0;
    size_t out_len = 0;

    while (1) {
        /* Skip leading whitespace? RFC 4180 does not allow leading whitespace,
           but we handle the standard case. */
        if (*p == '\0') {
            /* End of line */
            if (field_idx == idx) {
                out[out_len] = '\0';
                return (int)out_len;
            }
            return -1;
        }

        if (*p == '"') {
            /* Quoted field */
            p++;
            while (*p != '\0') {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        /* Doubled quote -> single quote */
                        if (out_len < cap - 1) {
                            out[out_len++] = '"';
                        }
                        p += 2;
                    } else {
                        /* Closing quote */
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
            /* After closing quote, expect comma or end of line */
            if (*p == ',') {
                p++;
                field_idx++;
                continue;
            } else if (*p == '\0') {
                if (field_idx == idx) {
                    out[out_len] = '\0';
                    return (int)out_len;
                }
                return -1;
            } else {
                /* Malformed: non-comma, non-NUL after closing quote.
                   Treat as end of field. */
                if (field_idx == idx) {
                    out[out_len] = '\0';
                    return (int)out_len;
                }
                return -1;
            }
        } else {
            /* Unquoted field */
            while (*p != '\0' && *p != ',') {
                if (out_len < cap - 1) {
                    out[out_len++] = *p;
                }
                p++;
            }
            if (*p == ',') {
                p++;
                field_idx++;
                continue;
            } else {
                /* End of line */
                if (field_idx == idx) {
                    out[out_len] = '\0';
                    return (int)out_len;
                }
                return -1;
            }
        }
    }
}