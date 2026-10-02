#include <stddef.h>
#include <ctype.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    const char *p = line;
    int field_idx = 0;
    int in_quotes = 0;
    size_t out_idx = 0;
    char c;

    while (*p != '\0' && *p != '\n' && *p != '\r') {
        if (field_idx == idx) {
            if (in_quotes) {
                p++;
                while (*p != '\0' && *p != '\n' && *p != '\r') {
                    if (*p == '"') {
                        if (*(p + 1) == '"') {
                            if (out_idx < cap - 1) out[out_idx++] = '"';
                            p += 2;
                        } else {
                            in_quotes = 0;
                            p++;
                        }
                    } else {
                        if (out_idx < cap - 1) out[out_idx++] = *p++;
                        else {
                            p++;
                            break;
                        }
                    }
                }
            } else {
                while (*p != '\0' && *p != '\n' && *p != '\r' && *p != ',') {
                    if (out_idx < cap - 1) out[out_idx++] = *p++;
                    else {
                        p++;
                        break;
                    }
                }
            }
            if (out_idx < cap) out[out_idx] = '\0';
            return (int)out_idx;
        }

        if (*p == '"') {
            in_quotes = !in_quotes;
            p++;
        } else {
            while (*p != '\0' && *p != '\n' && *p != '\r' && *p != ',') {
                p++;
            }
            if (*p == ',') {
                field_idx++;
                p++;
            }
        }
    }

    return -1;
}