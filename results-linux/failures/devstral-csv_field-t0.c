#include <string.h>
#include <ctype.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (cap == 0) return -1;
    const char *p = line;
    int field = 0;
    int in_quotes = 0;
    size_t out_pos = 0;

    while (*p && field <= idx) {
        if (in_quotes) {
            if (*p == '"') {
                const char *next = p + 1;
                if (*next == '"') {
                    if (out_pos < cap - 1) out[out_pos++] = '"';
                    p = next + 1;
                } else {
                    in_quotes = 0;
                    p++;
                }
            } else {
                if (out_pos < cap - 1) out[out_pos++] = *p;
                p++;
            }
        } else {
            if (*p == '"') {
                in_quotes = 1;
                p++;
            } else if (*p == ',') {
                field++;
                p++;
                if (field > idx) break;
            } else {
                if (out_pos < cap - 1) out[out_pos++] = *p;
                p++;
            }
        }
    }

    if (field != idx) return -1;
    if (out_pos < cap) out[out_pos] = '\0';
    return (int)out_pos;
}