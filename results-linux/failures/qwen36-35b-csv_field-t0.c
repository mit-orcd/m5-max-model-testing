#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) {
        return -1;
    }

    const char *p = line;
    int current_idx = 0;

    while (current_idx < idx) {
        // Skip current field
        if (*p == '"') {
            // Quoted field
            p++;
            while (*p != '\0') {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        p += 2; // Skip escaped quote
                    } else {
                        p++; // End of quoted field
                        break;
                    }
                } else {
                    p++;
                }
            }
        } else {
            // Unquoted field
            while (*p != '\0' && *p != ',') {
                p++;
            }
        }

        // Skip comma if not at end
        if (*p == ',') {
            p++;
        } else {
            // End of line before reaching idx
            return -1;
        }

        current_idx++;
    }

    // Now p points to the start of field idx
    size_t len = 0;
    int in_quotes = 0;

    if (*p == '"') {
        in_quotes = 1;
        p++;
    }

    while (*p != '\0' && *p != ',') {
        if (in_quotes) {
            if (*p == '"') {
                if (*(p + 1) == '"') {
                    if (len < cap - 1) {
                        out[len++] = '"';
                    }
                    p += 2;
                } else {
                    // End of quoted field
                    p++;
                    break;
                }
            } else {
                if (len < cap - 1) {
                    out[len++] = *p;
                }
                p++;
            }
        } else {
            if (len < cap - 1) {
                out[len++] = *p;
            }
            p++;
        }
    }

    out[len] = '\0';
    return (int)len;
}