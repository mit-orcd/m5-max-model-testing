#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    size_t out_idx = 0;
    int current_field = 0;
    int in_quotes = 0;
    size_t field_len = 0;

    if (idx < 0) return -1;
    if (cap == 0) {
        // Even if cap is 0, we must count the length to return it, but not write anything.
        // However, the spec says "truncated to cap-1", implying cap must be > 0 to store.
        // We still need to parse to find the length.
    }

    for (const char *p = line; *p != '\0'; ++p) {
        char c = *p;

        if (in_quotes) {
            if (c == '"') {
                if (*(p + 1) == '"') {
                    // Escaped quote
                    if (out_idx < cap - 1) {
                        out[out_idx++] = '"';
                    }
                    field_len++;
                    p++; // Skip next quote
                } else {
                    // End of quoted field
                    in_quotes = 0;
                }
            } else {
                if (out_idx < cap - 1) {
                    out[out_idx++] = c;
                }
                field_len++;
            }
        } else {
            if (c == '"') {
                in_quotes = 1;
            } else if (c == ',') {
                if (current_field == idx) {
                    if (out_idx < cap) {
                        out[out_idx] = '\0';
                    }
                    return (int)field_len;
                }
                current_field++;
                field_len = 0;
                out_idx = 0;
            } else {
                if (out_idx < cap - 1) {
                    out[out_idx++] = c;
                }
                field_len++;
            }
        }
    }

    // End of line reached
    if (current_field == idx) {
        if (out_idx < cap) {
            out[out_idx] = '\0';
        }
        return (int)field_len;
    }

    return -1;
}