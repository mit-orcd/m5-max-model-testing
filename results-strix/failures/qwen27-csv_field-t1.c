#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (cap == 0) return -1;

    int field = 0;
    size_t len = 0;
    const char *p = line;

    while (1) {
        /* Skip leading whitespace? RFC 4180 doesn't require it, but some parsers do.
           Standard RFC 4180: fields are separated by commas, no whitespace skipping. */
        
        /* Check if field is quoted */
        if (*p == '"') {
            /* Quoted field */
            p++; /* skip opening quote */
            while (*p != '\0') {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        /* Doubled quote: unescape to single quote */
                        if (len < cap - 1) {
                            out[len++] = '"';
                        }
                        p += 2;
                    } else {
                        /* Closing quote */
                        p++;
                        break;
                    }
                } else {
                    if (len < cap - 1) {
                        out[len++] = *p;
                    }
                    p++;
                }
            }
            /* After closing quote, expect comma or end of line */
            if (*p == ',') {
                p++;
            }
            /* If not comma and not end of line, it's malformed; but we proceed. */
            if (field == idx) {
                out[len] = '\0';
                return (int)len;
            }
            field++;
            if (*p == '\0') {
                return -1;
            }
        } else {
            /* Unquoted field */
            while (*p != '\0' && *p != ',') {
                if (len < cap - 1) {
                    out[len++] = *p;
                }
                p++;
            }
            if (*p == ',') {
                p++;
            }
            if (field == idx) {
                out[len] = '\0';
                return (int)len;
            }
            field++;
            if (*p == '\0') {
                return -1;
            }
        }
    }
}