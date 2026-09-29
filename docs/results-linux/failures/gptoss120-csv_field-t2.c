#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap)
{
    if (idx < 0) return -1;

    const size_t line_len = strlen(line);
    const int trailing_comma = line_len && line[line_len - 1] == ',';

    const char *p = line;
    int cur = 0;                     /* current field index */

    while (*p != '\0') {
        int quoted = (*p == '"');
        if (quoted) p++;            /* skip opening quote */

        size_t field_len = 0;       /* unescaped length of this field */
        size_t out_written = 0;     /* bytes written to out (may exceed cap-1) */

        while (1) {
            if (quoted) {
                if (*p == '\0') break;          /* unexpected end inside quotes */

                if (*p == '"') {
                    if (p[1] == '"') {          /* escaped quote */
                        if (cur == idx) {
                            if (out_written + 1 < cap)
                                out[out_written] = '"';
                            out_written++;
                        }
                        field_len++;
                        p += 2;
                        continue;
                    } else {                    /* closing quote */
                        p++;                     /* consume it */
                        break;
                    }
                } else {                        /* regular character inside quotes */
                    if (cur == idx) {
                        if (out_written + 1 < cap)
                            out[out_written] = *p;
                        out_written++;
                    }
                    field_len++;
                    p++;
                }
            } else {                            /* non‑quoted field */
                if (*p == ',' || *p == '\0')
                    break;
                if (cur == idx) {
                    if (out_written + 1 < cap)
                        out[out_written] = *p;
                    out_written++;
                }
                field_len++;
                p++;
            }
        }

        /* consume delimiter if present */
        if (*p == ',')
            p++;

        if (cur == idx) {
            if (cap > 0) {
                if (out_written < cap)
                    out[out_written] = '\0';
                else
                    out[cap - 1] = '\0';
            }
            return (int)field_len;
        }

        cur++;
    }

    /* handle possible trailing empty field after a final comma */
    if (trailing_comma && cur == idx) {
        if (cap > 0)
            out[0] = '\0';
        return 0;
    }

    return -1;   /* idx out of range */
}