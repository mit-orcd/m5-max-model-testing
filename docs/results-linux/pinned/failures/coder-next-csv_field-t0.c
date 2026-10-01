#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) {
        return -1;
    }

    const char *p = line;
    int field = 0;
    int in_quotes = 0;
    size_t out_pos = 0;

    while (*p != '\0') {
        if (field == idx) {
            if (out_pos < cap - 1) {
                out[out_pos++] = *p;
            }
            if (*p == '"') {
                if (p[1] == '"') {
                    p++;
                    if (out_pos < cap - 1) {
                        out[out_pos++] = '"';
                    }
                } else {
                    in_quotes = !in_quotes;
                }
            }
        } else {
            if (*p == '"') {
                if (p[1] == '"') {
                    p++;
                } else {
                    in_quotes = !in_quotes;
                }
            }
        }

        if (!in_quotes && *p == ',') {
            if (field == idx) {
                break;
            }
            field++;
            p++;
            continue;
        }

        p++;
    }

    if (field == idx) {
        out[out_pos] = '\0';
        return (int)out_pos;
    } else {
        return -1;
    }
}