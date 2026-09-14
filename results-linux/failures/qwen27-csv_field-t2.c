#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap)
{
    if (cap == 0)
        return -1;

    const char *p = line;
    int cur = 0;

    while (1) {
        /* Skip leading whitespace? RFC-4180 does not require it; skip. */
        if (*p == '"') {
            /* Quoted field */
            p++;
            size_t len = 0;
            for (;;) {
                if (*p == '\0') {
                    /* Unterminated quote; treat rest as field */
                    if (len < cap - 1)
                        out[len++] = *p;
                    break;
                }
                if (*p == '"') {
                    if (p[1] == '"') {
                        /* Doubled quote */
                        if (len < cap - 1)
                            out[len++] = '"';
                        p += 2;
                    } else {
                        /* Closing quote */
                        p++;
                        break;
                    }
                } else {
                    if (len < cap - 1)
                        out[len++] = *p;
                    p++;
                }
            }
            out[len] = '\0';
            if (cur == idx)
                return (int)len;
            cur++;
            /* Expect comma or end */
            if (*p == ',') {
                p++;
            } else {
                break;
            }
        } else {
            /* Unquoted field */
            size_t len = 0;
            while (*p != '\0' && *p != ',') {
                if (len < cap - 1)
                    out[len++] = *p;
                p++;
            }
            out[len] = '\0';
            if (cur == idx)
                return (int)len;
            cur++;
            if (*p == ',') {
                p++;
            } else {
                break;
            }
        }
    }

    return -1;
}