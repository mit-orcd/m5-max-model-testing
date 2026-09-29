#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap)
{
    if (cap == 0)
        return -1;

    const char *p = line;
    int field_idx = 0;

    for (;;) {
        /* Skip to the start of the current field */
        int quoted = 0;
        if (*p == '"') {
            quoted = 1;
            p++;
        }

        size_t out_len = 0;
        int in_quotes = quoted;

        while (*p) {
            if (in_quotes) {
                if (*p == '"') {
                    if (p[1] == '"') {
                        /* doubled quote -> single quote */
                        if (out_len < cap - 1)
                            out[out_len++] = '"';
                        p += 2;
                    } else {
                        /* closing quote */
                        in_quotes = 0;
                        p++;
                        /* After closing quote, the next char should be ',' or '\0' or '\r' */
                        if (*p == '\r') {
                            p++;
                        }
                        if (*p == ',') {
                            p++;
                            break;
                        } else if (*p == '\0') {
                            break;
                        } else {
                            /* Malformed: non-comma, non-NUL after closing quote.
                               Per RFC 4180 this is invalid, but we'll treat the rest
                               as literal until next comma or end. */
                            /* Continue parsing as unquoted */
                            in_quotes = 0;
                            /* don't consume the unexpected char; fall through to unquoted handling */
                            goto unquoted;
                        }
                    }
                } else {
                    if (out_len < cap - 1)
                        out[out_len++] = *p;
                    p++;
                }
            } else {
                if (*p == ',') {
                    p++;
                    break;
                } else if (*p == '\0') {
                    break;
                } else if (*p == '\r') {
                    p++;
                    if (*p == '\n')
                        p++;
                    break;
                } else {
unquoted:
                    if (out_len < cap - 1)
                        out[out_len++] = *p;
                    p++;
                }
            }
        }

        /* If we reached end of line, this is the last field */
        if (*p == '\0') {
            out[out_len] = '\0';
            return (int)out_len;
        }

        /* We consumed a comma; move to next field */
        field_idx++;
        if (field_idx == idx) {
            out[out_len] = '\0';
            return (int)out_len;
        }
        if (field_idx > idx) {
            break;
        }
    }

    return -1;
}