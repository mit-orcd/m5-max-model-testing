#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) {
        return -1;
    }

    const char *p = line;
    int field_idx = 0;

    while (1) {
        /* Skip leading whitespace? RFC 4180 doesn't require it, but let's be strict. */
        /* Actually, RFC 4180 fields may start with a quote or unquoted content. */
        
        size_t out_pos = 0;
        int in_quotes = 0;
        int field_ended = 0;

        if (*p == '"') {
            in_quotes = 1;
            p++;
        }

        while (1) {
            if (in_quotes) {
                if (*p == '"') {
                    if (p[1] == '"') {
                        /* Escaped quote */
                        if (out_pos + 1 < cap) {
                            out[out_pos++] = '"';
                        }
                        p += 2;
                    } else {
                        /* Closing quote */
                        in_quotes = 0;
                        p++;
                        /* After closing quote, next char should be comma or end of line */
                        if (*p == ',') {
                            p++;
                            field_ended = 1;
                            break;
                        } else if (*p == '\0') {
                            field_ended = 1;
                            break;
                        } else {
                            /* Malformed, but let's treat as end of field for robustness? 
                               Or continue? RFC says data after closing quote is invalid.
                               We'll stop here. */
                            field_ended = 1;
                            break;
                        }
                    }
                } else if (*p == '\0') {
                    /* Unterminated quote, treat rest as field content */
                    field_ended = 1;
                    break;
                } else {
                    if (out_pos + 1 < cap) {
                        out[out_pos++] = *p;
                    }
                    p++;
                }
            } else {
                /* Not in quotes */
                if (*p == ',') {
                    p++;
                    field_ended = 1;
                    break;
                } else if (*p == '\0') {
                    field_ended = 1;
                    break;
                } else {
                    if (out_pos + 1 < cap) {
                        out[out_pos++] = *p;
                    }
                    p++;
                }
            }
        }

        out[out_pos] = '\0';

        if (field_idx == idx) {
            return (int)out_pos;
        }

        if (!field_ended) {
            return -1;
        }

        field_idx++;
    }
}