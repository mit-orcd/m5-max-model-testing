#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap)
{
    if (line == NULL || out == NULL || cap == 0)
        return -1;

    int cur = 0;
    int in_quotes = 0;
    size_t len = 0;
    int escaped = 0; /* previous char was a quote inside quoted field */

    while (line[cur] != '\0') {
        char c = line[cur];

        if (in_quotes) {
            if (c == '"') {
                if (escaped && line[cur + 1] == '"') {
                    /* doubled quote: emit one quote */
                    if (len + 1 < cap)
                        out[len++] = '"';
                    cur++;
                } else {
                    in_quotes = 0;
                    escaped = 0;
                }
            } else {
                if (len + 1 < cap)
                    out[len++] = c;
                escaped = 0;
            }
        } else {
            if (c == '"') {
                in_quotes = 1;
                escaped = 0;
            } else if (c == ',') {
                if (cur_field == idx) {
                    out[len] = '\0';
                    return (int)len;
                }
                cur_field++;
                len = 0;
            } else {
                if (len + 1 < cap)
                    out[len++] = c;
            }
        }
        cur++;
    }

    /* End of line: handle last field */
    if (cur_field == idx) {
        out[len] = '\0';
        return (int)len;
    }
    return -1;
}