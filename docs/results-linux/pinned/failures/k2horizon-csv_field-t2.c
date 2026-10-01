#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) return -1;
    if (idx < 0) return -1;

    const char *p = line;
    int cur = 0;

    while (*p != '\0') {
        if (*p == '"') {
            /* quoted field */
            ++p;
            size_t len = 0;
            while (*p != '\0' && *p != '"') {
                if (*p == '"') {
                    if (p[1] == '"') {
                        if (len + 1 < cap) out[len++] = '"';
                        p += 2;
                    } else {
                        ++p; /* closing quote */
                        break;
                    }
                } else {
                    if (len + 1 < cap) out[len++] = *p;
                    ++p;
                }
            }
            /* skip to end of field */
            while (*p != '\0' && *p != ',') ++p;
            if (*p == ',') ++p;
            if (cur == idx) {
                if (len + 1 < cap) out[len] = '\0';
                else out[cap - 1] = '\0';
                return (int)len;
            }
            ++cur;
        } else {
            /* unquoted field */
            const char *start = p;
            while (*p != '\0' && *p != ',') ++p;
            size_t len = (size_t)(p - start);
            if (cur == idx) {
                if (len + 1 < cap) {
                    for (size_t i = 0; i < len; ++i) out[i] = start[i];
                    out[len] = '\0';
                } else {
                    for (size_t i = 0; i < cap - 1; ++i) out[i] = start[i];
                    out[cap - 1] = '\0';
                }
                return (int)len;
            }
            if (*p == ',') ++p;
            ++cur;
        }
    }

    return -1;
}