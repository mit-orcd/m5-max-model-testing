#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (idx < 0 || cap == 0) return -1;

    int field = 0;
    size_t out_len = 0;
    int quoted = 0;

    while (*line) {
        if (field == idx) {
            if (quoted) {
                if (*line == '"') {
                    if (line[1] == '"') {
                        if (out_len + 1 < cap - 1) {
                            out[out_len++] = '"';
                        }
                        line += 2;
                        continue;
                    } else {
                        quoted = 0;
                        line++;
                        if (*line == ',') {
                            break;
                        }
                        if (*line == '\0') {
                            break;
                        }
                        continue;
                    }
                } else {
                    if (out_len + 1 < cap - 1) {
                        out[out_len++] = *line;
                    }
                    line++;
                    continue;
                }
            } else {
                if (*line == ',') {
                    break;
                }
                if (*line == '"') {
                    quoted = 1;
                    line++;
                    continue;
                }
                if (out_len + 1 < cap - 1) {
                    out[out_len++] = *line;
                }
                line++;
                continue;
            }
        }

        if (quoted) {
            if (*line == '"') {
                if (line[1] == '"') {
                    line += 2;
                    continue;
                } else {
                    quoted = 0;
                    line++;
                    continue;
                }
            } else {
                line++;
                continue;
            }
        } else {
            if (*line == ',') {
                field++;
                line++;
                continue;
            }
            line++;
            continue;
        }
    }

    if (field != idx) return -1;

    out[out_len] = '\0';
    return (int)out_len;
}