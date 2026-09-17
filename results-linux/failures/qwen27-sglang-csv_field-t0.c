#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) {
        return -1;
    }

    const char *p = line;
    int field_idx = 0;

    while (1) {
        /* Skip leading whitespace? RFC 4180 doesn't specify, but typically no.
           We'll just process as-is. */
        
        int in_quotes = 0;
        size_t len = 0;
        
        if (*p == '"') {
            in_quotes = 1;
            p++;
            while (*p != '\0') {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        /* Escaped quote */
                        if (len < cap - 1) {
                            out[len++] = '"';
                        }
                        p += 2;
                    } else {
                        /* End of quoted field */
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
            /* If not comma and not end, it's malformed, but we'll just proceed */
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

        if (field_idx == idx) {
            out[len] = '\0';
            return (int)len;
        }

        field_idx++;

        /* If we've reached the end of the line, no more fields */
        if (*p == '\0') {
            return -1;
        }
    }
}