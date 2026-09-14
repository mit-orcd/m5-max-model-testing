#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    const char *p = line;
    int current = 0;
    size_t len = 0;
    char quote = 0;
    int escaped = 0;

    if (idx < 0) {
        return -1;
    }

    while (*p != '\0') {
        if (current == idx && !quote) {
            break;
        }

        if (*p == '"') {
            if (current == idx && quote == 0 && len < cap - 1) {
                *out++ = *p++;
                len++;
            } else {
                p++;
                if (*p == '"') {
                    if (current == idx && quote && len < cap - 1) {
                        *out++ = *p++;
                        len++;
                    } else {
                        p++;
                    }
                } else {
                    quote = !quote;
                }
            }
        } else if (*p == ',' && !quote) {
            p++;
            current++;
        } else {
            if (current == idx) {
                if (len < cap - 1) {
                    *out++ = *p++;
                    len++;
                } else {
                    out[len] = '\0';
                    return len;
                }
            } else {
                p++;
            }
        }
    }

    if (current == idx) {
        out[len] = '\0';
        return len;
    } else {
        return -1;
    }
}