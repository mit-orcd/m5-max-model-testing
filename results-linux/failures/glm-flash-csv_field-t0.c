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
                while (*p != '\0') {
                    if (*p == '"') {
                        if (*(p + 1) == '"') {
                            p++;
                        } else {
                            break;
                        }
                    }
                    p++;
                }
                end = p;
                break;
            } else {
                start = p;
                while (*p != '\0' && *p != ',') {
                    p++;
                }
                end = p;
                break;
            }
        }

        if (*p == '"') {
            in_quotes = !in_quotes;
            p++;
        } else if (*p == ',' && !in_quotes) {
            current++;
            p++;
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

    size_t copy_len = end - start;
    if (copy_len > cap - 1) {
        copy_len = cap - 1;
    }

    memcpy(out, start, copy_len);
    out[copy_len] = '\0';

    return (int)copy_len;
}