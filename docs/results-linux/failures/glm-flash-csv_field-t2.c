#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    int current = 0;
    size_t out_len = 0;
    int is_quoted = 0;

    while (*line) {
        if (*line == '"') {
            if (is_quoted) {
                const char *next = line + 1;
                if (*next == '"') {
                    // Double quote: treat as single quote and advance past both
                    if (out_len < cap) {
                        out[out_len++] = '"';
                    }
                    line = next + 1;
                } else {
                    // End of quoted field
                    is_quoted = 0;
                    line++;
                }
            } else {
                // Start of quoted field
                is_quoted = 1;
                line++;
            }
        } else {
            // Normal character or end of quoted field
            if (*line == ',') {
                if (!is_quoted) {
                    // End of field
                    if (current == idx) {
                        out[out_len] = '\0';
                        return (int)out_len;
                    }
                    current++;
                    out_len = 0;
                }
                line++;
            } else {
                // Regular character
                if (out_len < cap) {
                    out[out_len++] = *line;
                }
                line++;
            }
        }
    }

    // Handle the last field (which might be quoted or unquoted)
    if (current == idx) {
        out[out_len] = '\0';
        return (int)out_len;
    }

    return -1;
}