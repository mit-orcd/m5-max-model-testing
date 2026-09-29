#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL) return -1;
    if (idx < 0) return -1;

    const char *p = line;
    int current = 0;
    size_t len = 0;
    size_t o = 0;
    int in_quotes = 0;
    int found = 0;

    while (*p) {
        if (current == idx) {
            found = 1;
            if (!in_quotes) {
                if (*p == '"') {
                    in_quotes = 1;
                    p++;
                    continue;
                }
            }

            if (in_quotes) {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        // Escaped quote
                        if (o < cap - 1) out[o++] = '"';
                        len++;
                        p += 2;
                        continue;
                    } else {
                        // End of quoted field
                        in_quotes = 0;
                        p++;
                        continue;
                    }
                } else {
                    if (o < cap - 1) out[o++] = *p;
                    len++;
                    p++;
                    continue;
                }
            } else {
                // Unquoted field
                if (*p == ',' || *p == '\0') {
                    // End of field
                    if (o < cap) out[o] = '\0';
                    return (int)len;
                }
                if (o < cap - 1) out[o++] = *p;
                len++;
                p++;
            }
        } else {
            // Skip to next field
            if (!in_quotes) {
                if (*p == '"') {
                    in_quotes = 1;
                    p++;
                    continue;
                }
                if (*p == ',') {
                    current++;
                    if (current == idx) continue; // Move to processing logic
                    p++;
                    continue;
                }
            } else {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        p += 2;
                        continue;
                    } else {
                        in_quotes = 0;
                        p++;
                        continue;
                    }
                }
            }
            p++;
        }
    }

    // Handle end of string
    if (current == idx && !in_quotes) {
        if (o < cap) out[o] = '\0';
        return (int)len;
    }

    // Check if we reached the end but still need the last field
    if (current == idx && in_quotes) {
        // Malformed: unclosed quote, but treat as end
        if (o < cap) out[o] = '\0';
        return (int)len;
    }

    if (!found) return -1;
    return (int)len;
}