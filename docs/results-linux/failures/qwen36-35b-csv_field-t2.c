#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) {
        return -1;
    }

    const char *p = line;
    int current_idx = 0;

    // Skip to the start of the desired field
    while (current_idx < idx) {
        // Skip current field
        if (*p == '"') {
            // Quoted field
            p++;
            while (*p != '\0') {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        p += 2; // Escaped quote
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

        // Skip comma if not end of line
        if (*p == ',') {
            p++;
            current_idx++;
        } else {
            // End of line before reaching idx
            return -1;
        }
    }

    // Now p points to the start of field idx
    if (*p == '\0') {
        // Empty field at end or only one field
        if (idx == current_idx) {
            if (cap > 0) {
                out[0] = '\0';
            }
            return 0;
        }
        return -1;
    }

    size_t len = 0;
    int in_quotes = 0;

    while (*p != '\0') {
        if (*p == ',') {
            if (!in_quotes) {
                break;
            }
        } else if (*p == '"') {
            if (in_quotes) {
                if (*(p + 1) == '"') {
                    // Escaped quote
                    if (len < cap - 1) {
                        out[len++] = '"';
                    }
                    p++; // Skip next quote
                } else {
                    // End of quoted field
                    in_quotes = 0;
                }
            } else {
                // Start of quoted field
                in_quotes = 1;
            }
        } else {
            if (len < cap - 1) {
                out[len++] = *p;
            }
        }
        p++;
    }

    out[len] = '\0';
    return (int)len;
}