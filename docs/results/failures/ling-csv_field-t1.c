#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || idx < 0 || !out || cap == 0) return -1;

    int field = 0;
    size_t out_len = 0;
    int quoted = 0;

    while (*line) {
        if (field == idx) {
            if (quoted) {
                if (*line == '"') {
                    if (line[1] == '"') {
                        if (out_len < cap - 1) out[out_len++] = '"';
                        line += 2;
                    } else {
                        quoted = 0;
                        line++;
                    }
                } else {
                    if (out_len < cap - 1) out[out_len++] = *line;
                    line++;
                }
            } else {
                if (*line == ',') break;
                if (*line == '"') {
                    quoted = 1;
                    line++;
                } else {
                    if (out_len < cap - 1) out[out_len++] = *line;
                    line++;
                }
            }
        } else {
            if (quoted) {
                if (*line == '"') {
                    if (line[1] == '"') {
                        line += 2;
                    } else {
                        quoted = 0;
                        line++;
                    }
                } else {
                    line++;
                }
            } else {
                if (*line == ',') {
                    field++;
                    line++;
                } else {
                    line++;
                }
            }
        }
    }

    if (field != idx) return -1;

    out[out_len] = '\0';
    return (int)out_len;
}