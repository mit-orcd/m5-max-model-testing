#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) {
        return -1;
    }

    const char *p = line;
    int field_idx = 0;

    while (1) {
        /* Skip leading whitespace? RFC 4180 doesn't specify, but typically no.
           We just process the field as-is. */
        
        size_t len = 0;
        int in_quotes = 0;

        if (*p == '"') {
            in_quotes = 1;
            p++;
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
            /* If not comma and not end, it's malformed; we'll just continue */
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
        }

        /* NUL-terminate */
        out[len] = '\0';

        if (field_idx == idx) {
            return (int)len;
        }

        /* If we've reached the end of the line, no more fields */
        if (*p == '\0') {
            return -1;
        }

        field_idx++;
    }
}