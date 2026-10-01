#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    const char *p = line;
    const char *start = NULL;
    const char *end = NULL;
    int current = 0;
    int in_quotes = 0;
    size_t out_len = 0;

    if (idx < 0) {
        return -1;
    }

    while (*p != '\0') {
        if (current == idx) {
            if (in_quotes) {
                start = p;
            } else {
                start = p;
                while (*p != '\0' && *p != ',' && *p != '\n') {
                    p++;
                }
                end = p;
                break;
            }
        }

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
        } else if (*p == ',' && !in_quotes) {
            current++;
        }

        p++;
    }

    if (current != idx) {
        return -1;
    }

    if (in_quotes) {
        return -1;
    }

    if (start == NULL) {
        return -1;
    }

    if (end == NULL) {
        end = p;
    }

    size_t copy_len = end - start;
    if (copy_len > cap - 1) {
        copy_len = cap - 1;
    }

    memcpy(out, start, copy_len);
    out[copy_len] = '\0';

    return (int)copy_len;
}