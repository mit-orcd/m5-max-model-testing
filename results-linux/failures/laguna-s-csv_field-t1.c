#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    const char *p = line;
    int current = 0;
    size_t out_len = 0;

    while (current <= idx && *p != '\0') {
        if (*p == '"') {
            // Quoted field
            if (current == idx) {
                p++; // skip opening quote
                while (*p != '\0' && out_len < cap - 1) {
                    if (*p == '"') {
                        if (*(p + 1) == '"') {
                            // Escaped quote
                            if (out_len < cap - 1) {
                                out[out_len++] = '"';
                            }
                            p += 2;
                        } else {
                            // End of quoted field
                            p++;
                            break;
                        }
                    } else {
                        out[out_len++] = *p++;
                    }
                }
                out[out_len] = '\0';
                // Skip to end of field (past closing quote and any comma)
                while (*p != '\0' && *p != ',') p++;
                if (*p == ',') p++;
                return (int)out_len;
            } else {
                // Skip quoted field
                p++; // skip opening quote
                while (*p != '\0') {
                    if (*p == '"') {
                        if (*(p + 1) == '"') {
                            p += 2;
                        } else {
                            p++;
                            break;
                        }
                    } else {
                        p++;
                    }
                }
                if (*p == ',') p++;
                current++;
            }
        } else {
            // Unquoted field
            const char *start = p;
            while (*p != '\0' && *p != ',') p++;
            if (current == idx) {
                size_t len = (size_t)(p - start);
                if (len > cap - 1) len = cap - 1;
                memcpy(out, start, len);
                out[len] = '\0';
                if (*p == ',') p++;
                return (int)len;
            }
            if (*p == ',') p++;
            current++;
        }
    }

    return -1;
}