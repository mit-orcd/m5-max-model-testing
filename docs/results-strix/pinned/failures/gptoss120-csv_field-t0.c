#include <stddef.h>
#include <stdbool.h>

int csv_field(const char *line, int idx, char *out, size_t cap)
{
    if (!line || idx < 0) return -1;

    size_t out_written = 0;   /* bytes actually stored (may be truncated) */
    size_t field_len   = 0;   /* unescaped length of the target field   */
    int cur_idx = 0;
    const char *p = line;

    while (true) {
        bool capture = (cur_idx == idx);

        if (*p == '"') {                     /* quoted field */
            ++p;                             /* skip opening quote */
            while (true) {
                if (*p == '\0') {           /* malformed, treat as end */
                    break;
                }
                if (*p == '"') {
                    if (p[1] == '"') {      /* escaped quote */
                        if (capture && out_written + 1 < cap) out[out_written] = '"';
                        ++out_written;
                        ++field_len;
                        p += 2;
                        continue;
                    } else {               /* closing quote */
                        ++p;
                        break;
                    }
                } else {                   /* regular character */
                    if (capture && out_written + 1 < cap) out[out_written] = *p;
                    ++out_written;
                    ++field_len;
                    ++p;
                }
            }
            if (*p == ',') ++p;             /* skip field separator */
        } else {                             /* unquoted field */
            while (*p != ',' && *p != '\0') {
                if (capture && out_written + 1 < cap) out[out_written] = *p;
                ++out_written;
                ++field_len;
                ++p;
            }
            if (*p == ',') ++p;             /* skip field separator */
        }

        if (capture) {                      /* target field processed */
            break;
        }

        ++cur_idx;
        if (*p == '\0') {                   /* no more fields */
            break;
        }
    }

    /* If we never captured the requested index, it is out of range */
    if (cur_idx != idx) {
        return -1;
    }

    /* NUL‑terminate the output buffer */
    if (cap > 0) {
        size_t term = (out_written < cap) ? out_written : cap - 1;
        out[term] = '\0';
    }

    return (int)field_len;
}