#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    const char *p = line;
    const char *start = NULL;
    const char *end = NULL;
    int current = 0;
    int in_quotes = 0;
    size_t out_len = 0;

    if (idx < 0) return -1;

    while (*p != '\0') {
        if (current == idx) {
            start = p;
            while (*p != '\0' && *p != '\n' && *p != '\r') {
                if (*p == '"') {
                    if (in_quotes && *(p + 1) == '"') {
                        p++;
                    } else {
                        in_quotes = !in_quotes;
                    }
                }
                p++;
            }
            end = p;
            break;
        }

        if (*p == '\n' || *p == '\r') {
            current++;
            p++;
            while ((*p == '\n' || *p == '\r') && *p != '\0') {
                p++;
            }
        } else {
            p++;
        }
    }

    if (current != idx) {
        return -1;
    }

    if (start == NULL) {
        return -1;
    }

    p = start;
    while (p < end) {
        if (*p == '"') {
            if (in_quotes) {
                if (*(p + 1) == '"') {
                    p++;
                } else {
                    in_quotes = 0;
                }
            } else {
                in_quotes = 1;
            }
        }

        if (in_quotes) {
            if (out_len + 1 < cap) {
                out[out_len++] = *p;
            }
        } else {
            if (*p != ',') {
                if (out_len + 1 < cap) {
                    out[out_len++] = *p;
                }
            } else {
                break;
            }
        }
        p++;
    }

    if (out_len >= cap) {
        out[cap - 1] = '\0';
    } else {
        out[out_len] = '\0';
    }

    return (int)out_len;
}