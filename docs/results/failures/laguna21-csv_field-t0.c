#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (idx < 0 || !line || !out || cap == 0) return -1;

    const char *p = line;
    int current_idx = 0;
    char *dest = out;
    size_t remaining = cap;

    while (*p && current_idx < idx) {
        if (*p == '"') {
            p++;
            while (*p && *p != '"') {
                if (remaining > 1) {
                    *dest++ = *p;
                    remaining--;
                }
                p++;
            }
            if (*p == '"') p++;
        } else {
            while (*p && *p != ',') {
                if (remaining > 1) {
                    *dest++ = *p;
                    remaining--;
                }
                p++;
            }
        }
        if (*p == ',') p++;
        current_idx++;
    }

    if (current_idx != idx) return -1;

    if (*p == '"') {
        p++;
        while (*p && *p != '"') {
            if (remaining > 1) {
                if (*p == '"' && *(p+1) == '"') {
                    *dest++ = '"';
                    remaining--;
                    p += 2;
                } else {
                    *dest++ = *p;
                    remaining--;
                    p++;
                }
            } else {
                p++;
            }
        }
    } else {
        while (*p && *p != ',' && remaining > 1) {
            *dest++ = *p;
            remaining--;
            p++;
        }
    }

    *dest = '\0';
    return (int)(dest - out);
}