#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap)
{
    if (!line || idx < 0)
        return -1;

    /* Empty line = one empty field */
    if (*line == '\0') {
        if (idx == 0) {
            if (cap > 0) out[0] = '\0';
            return 0;
        }
        return -1;
    }

    const char *p = line;
    int cur = 0;
    int last_was_comma = 0;          /* true if we just consumed a ',' */
    while (1) {
        /* Detect trailing empty field after a final comma */
        if (*p == '\0' && !last_was_comma)
            break;                  /* no more fields */

        int quoted = (*p == '"');
        size_t field_len = 0;       /* unescaped length */
        size_t out_written = 0;     /* bytes actually stored */

        if (quoted) {
            p++;                     /* skip opening quote */
            while (*p) {
                if (*p == '"') {
                    if (*(p + 1) == '"') {          /* escaped quote */
                        if (out && out_written + 1 < cap)
                            out[out_written] = '"';
                        ++out_written;
                        ++field_len;
                        p += 2;
                    } else {                        /* closing quote */
                        ++p;                         /* skip it */
                        break;
                    }
                } else {
                    if (out && out_written + 1 < cap)
                        out[out_written] = *p;
                    ++out_written;
                    ++field_len;
                    ++p;
                }
            }
            if (*p == ',') {        /* consume field separator */
                ++p;
                last_was_comma = 1;
            } else {
                last_was_comma = 0;
            }
        } else {
            while (*p && *p != ',') {
                if (out && out_written + 1 < cap)
                    out[out_written] = *p;
                ++out_written;
                ++field_len;
                ++p;
            }
            if (*p == ',') {
                ++p;
                last_was_comma = 1;
            } else {
                last_was_comma = 0;
            }
        }

        if (cur == idx) {
            if (out && cap > 0) {
                size_t term = (out_written < cap) ? out_written : cap - 1;
                out[term] = '\0';
            }
            return (int)field_len;
        }

        ++cur;

        /* If we reached the string end after a comma, allow one more empty field */
        if (*p == '\0' && last_was_comma) {
            /* Process the trailing empty field on the next loop iteration */
            last_was_comma = 0;   /* reset so we exit after handling it */
            continue;
        }
    }

    return -1;   /* idx out of range */
}