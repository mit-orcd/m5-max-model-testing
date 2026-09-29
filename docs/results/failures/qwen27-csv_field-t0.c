#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) {
        return -1;
    }

    const char *p = line;
    int field_idx = 0;

    for (;;) {
        /* Skip leading whitespace? RFC 4180 doesn't specify, but typically no.
           We just start parsing the field. */
        
        int in_quotes = 0;
        size_t out_len = 0;
        
        if (*p == '"') {
            in_quotes = 1;
            p++;
        }
        
        while (*p != '\0') {
            if (in_quotes) {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        /* Escaped quote */
                        if (out_len < cap - 1) {
                            out[out_len++] = '"';
                        }
                        p += 2;
                    } else {
                        /* End of quoted field */
                        p++;
                        break;
                    }
                } else {
                    if (out_len < cap - 1) {
                        out[out_len++] = *p;
                    }
                    p++;
                }
            } else {
                if (*p == ',') {
                    p++;
                    break;
                } else if (*p == '\0') {
                    break;
                } else {
                    if (out_len < cap - 1) {
                        out[out_len++] = *p;
                    }
                    p++;
                }
            }
        }
        
        /* NUL terminate */
        out[out_len] = '\0';
        
        if (field_idx == idx) {
            return (int)out_len;
        }
        
        /* Move to next field */
        if (*p == '\0') {
            /* End of line, no more fields */
            return -1;
        }
        
        /* p should be at ',' or '\0' */
        if (*p == ',') {
            p++;
            field_idx++;
        } else {
            /* Unexpected state, but treat as end */
            return -1;
        }
    }
}