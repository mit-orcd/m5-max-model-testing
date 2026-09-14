#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (cap == 0) return -1;

    const char *p = line;
    int current = 0;

    while (current < idx) {
        // Skip empty fields or move to next field
        if (*p == '\0') return -1; // End of line before reaching idx

        if (*p == '"') {
            // Quoted field: skip until closing quote
            p++;
            while (*p != '\0') {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        p += 2; // Skip escaped quote
                    } else {
                        p++; // Skip closing quote
                        break;
                    }
                } else {
                    p++;
                }
            }
            if (*p == '\0') return -1; // Unterminated quote
            // Skip comma after field
            if (*p == ',') p++;
        } else {
            // Unquoted field: skip until comma or end
            while (*p != '\0' && *p != ',') {
                p++;
            }
            if (*p == ',') p++;
        }
        current++;
    }

    // Now p points to the start of field idx
    if (*p == '\0' && idx == 0) {
        // Empty line, idx 0 is empty string
        out[0] = '\0';
        return 0;
    }

    size_t len = 0;
    int in_quotes = 0;
    int escaped = 0;

    while (*p != '\0' && *p != ',') {
        if (in_quotes) {
            if (escaped) {
                if (*p == '"') {
                    out[len++] = '"';
                    escaped = 0;
                } else {
                    out[len++] = '\\';
                    out[len++] = *p;
                }
            } else {
                if (*p == '"') {
                    escaped = 1;
                } else {
                    out[len++] = *p;
                }
            }
        } else {
            if (*p == '"') {
                in_quotes = 1;
            } else {
                out[len++] = *p;
            }
        }
        p++;
    }

    if (len >= cap) {
        len = cap - 1;
    }
    out[len] = '\0';

    return (int)len;
}