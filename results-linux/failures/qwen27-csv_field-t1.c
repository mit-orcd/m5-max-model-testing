#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (idx < 0 || cap == 0) return -1;

    size_t out_len = 0;
    int field_idx = 0;
    const char *p = line;

    while (1) {
        /* Skip whitespace? RFC 4180 does not specify leading/trailing spaces.
         * We do NOT skip spaces as they are part of the field.
         * But we do need to handle the start of a field.
         */

        int in_quotes = 0;

        if (*p == '"') {
            in_quotes = 1;
            p++;
        }

        /* Now parse the field content */
        while (1) {
            if (in_quotes) {
                if (*p == '\0') {
                    /* Unterminated quote - treat rest as field? 
                       RFC 4180 says it's invalid, but we'll be lenient.
                       Actually, if we hit NUL, the field is complete (truncated input).
                       Break out.
                    */
                    break;
                }
                if (*p == '"') {
                    if (p[1] == '"') {
                        /* Escaped quote */
                        if (out_len < cap - 1) {
                            out[out_len++] = '"';
                        }
                        p += 2;
                        continue;
                    } else {
                        /* End of quoted field */
                        p++;
                        break;
                    }
                }
                /* Regular character inside quotes */
                if (out_len < cap - 1) {
                    out[out_len++] = *p;
                }
                p++;
            } else {
                /* Not in quotes */
                if (*p == '\0' || *p == ',' || *p == '\r' || *p == '\n') {
                    break;
                }
                if (out_len < cap - 1) {
                    out[out_len++] = *p;
                }
                p++;
            }
        }

        out[out_len] = '\0';

        /* Check if this was the field we wanted */
        if (field_idx == idx) {
            return (int)out_len;
        }

        /* Move to next field */
        if (*p == '\0') {
            /* No more fields */
            return -1;
        }
        if (*p == ',') {
            p++;
            field_idx++;
            continue;
        }
        if (*p == '\r' || *p == '\n') {
            /* End of line */
            return -1;
        }
        /* Should not reach here in valid CSV, but to be safe */
        return -1;
    }
}