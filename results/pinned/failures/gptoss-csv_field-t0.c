#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap)
{
    const char *p = line;
    int cur = 0;                 /* current field index */
    size_t out_len = 0;          /* number of chars written to out */
    size_t field_len = 0;        /* unescaped length of the target field */

    while (1) {
        int quoted = (*p == '"');
        if (quoted) {
            /* quoted field */
            p++; /* skip opening quote */
            while (*p) {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        /* escaped quote */
                        if (cur == idx && out_len + 1 < cap)
                            out[out_len++] = '"';
                        field_len++;
                        p += 2;
                    } else {
                        /* closing quote */
                        p++; /* skip closing quote */
                        break;
                    }
                } else {
                    if (cur == idx && out_len + 1 < cap)
                        out[out_len++] = *p;
                    field_len++;
                    p++;
                }
            }
            /* after quoted field, skip following comma if present */
            if (*p == ',')
                p++;
        } else {
            /* unquoted field */
            while (*p && *p != ',') {
                if (cur == idx && out_len + 1 < cap)
                    out[out_len++] = *p;
                field_len++;
                p++;
            }
            if (*p == ',')
                p++;
        }

        /* field processed */
        if (cur == idx)
            break;          /* we have extracted the requested field */

        cur++;
        if (*p == '\0')
            break;          /* end of line reached */
    }

    if (cur != idx)
        return -1;          /* idx out of range */

    if (cap > 0)
        out[out_len] = '\0';

    return (int)field_len;
}