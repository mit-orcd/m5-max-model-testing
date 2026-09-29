#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) {
        return -1;
    }

    const char *p = line;
    int field_num = 0;
    int in_quotes = 0;
    size_t out_pos = 0;

    while (*p != '\0' && field_num <= idx) {
        if (field_num == idx) {
            // Extract current field
            if (out_pos < cap - 1) {
                out[out_pos] = '\0';
            }
            return (int)out_pos;
        }

        if (*p == '"') {
            if (in_quotes) {
                if (p[1] == '"') {
                    // Escaped quote
                    if (out_pos < cap - 1) {
                        out[out_pos++] = '"';
                    }
                    p += 2;
                } else {
                    // End of quoted field
                    in_quotes = 0;
                    p++;
                }
            } else {
                // Start of quoted field
                in_quotes = 1;
                p++;
            }
        } else if (*p == ',' && !in_quotes) {
            // Field separator
            field_num++;
            p++;
        } else {
            // Regular character
            if (out_pos < cap - 1) {
                out[out_pos++] = *p;
            }
            p++;
        }
    }

    // Check if we reached the end of line with the target field
    if (field_num == idx) {
        if (out_pos < cap - 1) {
            out[out_pos] = '\0';
        }
        return (int)out_pos;
    }

    return -1;
}