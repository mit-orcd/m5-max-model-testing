#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) {
        return -1;
    }

    const char *p = line;
    int field_idx = 0;
    size_t out_len = 0;

    while (1) {
        /* Skip leading whitespace? RFC 4180 doesn't specify, but typically no.
           We just check if we're at a quoted field. */
        if (*p == '"') {
            /* Quoted field */
            p++; /* skip opening quote */
            while (1) {
                if (*p == '\0') {
                    /* Unterminated quote; treat rest as field content */
                    break;
                }
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
            }
            /* If not comma and not end, it's malformed; we'll just continue */
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
            }
        }

        /* We've parsed one field */
        if (field_idx == idx) {
            out[out_len] = '\0';
            return (int)out_len;
        }

        /* Check if we've reached end of line */
        if (*p == '\0') {
            return -1;
        }

        field_idx++;
    }
}