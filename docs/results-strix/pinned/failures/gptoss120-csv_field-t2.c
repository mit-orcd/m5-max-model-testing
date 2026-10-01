#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap)
{
    if (idx < 0) return -1;

    size_t write_pos = 0;      /* bytes actually written to out */
    size_t field_len = 0;      /* unescaped length of the target field */
    int cur = 0;               /* current field index */
    const char *p = line;

    while (1) {
        int is_target = (cur == idx);
        int quoted = (*p == '"');

        if (quoted) {
            ++p; /* skip opening quote */
            while (1) {
                char c = *p;
                if (c == '\0')               /* malformed, treat as end */
                    break;
                if (c == '"') {
                    if (p[1] == '"') {       /* escaped quote */
                        if (is_target && write_pos + 1 < cap)
                            out[write_pos] = '"';
                        if (write_pos + 1 < cap) ++write_pos;
                        ++field_len;
                        p += 2;
                        continue;
                    } else {                /* closing quote */
                        ++p;
                        break;
                    }
                } else {
                    if (is_target && write_pos + 1 < cap)
                        out[write_pos] = c;
                    if (write_pos + 1 < cap) ++write_pos;
                    ++field_len;
                    ++p;
                }
            }
            if (*p == ',')
                ++p;               /* move to next field */
        } else {
            while (*p != ',' && *p != '\0') {
                char c = *p;
                if (is_target && write_pos + 1 < cap)
                    out[write_pos] = c;
                if (write_pos + 1 < cap) ++write_pos;
                ++field_len;
                ++p;
            }
            if (*p == ',')
                ++p;               /* move to next field */
        }

        if (cur == idx)               /* target field processed */
            break;

        ++cur;                         /* advance to next field */

        if (*p == '\0')                /* no more fields */
            break;
    }

    if (cur != idx)                    /* idx out of range */
        return -1;

    if (cap > 0) {
        size_t term = (write_pos < cap) ? write_pos : cap - 1;
        out[term] = '\0';
    }

    return (int)field_len;
}