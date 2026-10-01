#include <string.h>
#include <ctype.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (cap == 0) return -1;
    const char *p = line;
    int field = 0;
    char *q = out;
    size_t len = 0;

    while (*p && field <= idx) {
        if (field == idx) {
            if (*p == '"') {
                p++;
                while (*p && len < cap - 1) {
                    if (*p == '"') {
                        p++;
                        if (*p == '"') {
                            if (q != out) *q++ = '"';
                            len++;
                            p++;
                        } else {
                            break;
                        }
                    } else {
                        *q++ = *p++;
                        len++;
                    }
                }
            } else {
                while (*p != ',' && *p != '\0' && len < cap - 1) {
                    *q++ = *p++;
                    len++;
                }
            }
            *q = '\0';
            return len;
        }

        if (*p == '"') {
            p++;
            while (*p && *p != '"') {
                if (*p == '"') p++;
                p++;
            }
            if (*p == '"') p++;
        } else {
            while (*p && *p != ',') p++;
        }

        if (*p == ',') p++;
        field++;
    }

    return -1;
}