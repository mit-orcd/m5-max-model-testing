#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap)
{
    if (!line || idx < 0)
        return -1;

    int cur = 0;               /* current field index */
    size_t pos = 0;            /* position in line */

    while (line[pos] != '\0') {
        int quoted = (line[pos] == '"');
        size_t field_len = 0;  /* unescaped length of this field */
        size_t out_pos = 0;    /* bytes written to out for this field */

        if (quoted) {
            ++pos;  /* skip opening quote */

            while (line[pos] != '\0') {
                if (line[pos] == '"') {
                    if (line[pos + 1] == '"') {          /* escaped quote */
                        if (cur == idx && out_pos + 1 < cap) {
                            out[out_pos] = '"';
                        }
                        ++out_pos;
                        ++field_len;
                        pos += 2;
                    } else {                               /* closing quote */
                        ++pos;
                        break;
                    }
                } else {                                   /* regular char */
                    if (cur == idx && out_pos + 1 < cap) {
                        out[out_pos] = line[pos];
                    }
                    ++out_pos;
                    ++field_len;
                    ++pos;
                }
            }

            if (line[pos] == ',')
                ++pos;   /* skip field separator */
        } else {   /* unquoted field */
            while (line[pos] != '\0' && line[pos] != ',') {
                if (cur == idx && out_pos + 1 < cap) {
                    out[out_pos] = line[pos];
                }
                ++out_pos;
                ++field_len;
                ++pos;
            }
            if (line[pos] == ',')
                ++pos;   /* skip field separator */
        }

        if (cur == idx) {
            if (cap > 0) {
                size_t term = (out_pos < cap) ? out_pos : cap - 1;
                out[term] = '\0';
            }
            return (int)field_len;
        }

        ++cur;   /* move to next field */
    }

    /* idx out of range */
    return -1;
}