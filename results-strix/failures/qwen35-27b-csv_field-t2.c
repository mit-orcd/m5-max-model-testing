#include <stddef.h>
#include <stdbool.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) {
        return -1;
    }

    const char *p = line;
    int current_idx = 0;
    size_t out_len = 0;

    while (true) {
        bool quoted = false;
        bool has_content = false;

        // Check for quote start
        if (*p == '"') {
            quoted = true;
            p++;
        }

        while (*p) {
            if (quoted) {
                if (*p == '"') {
                    // Check for escaped quote ""
                    if (*(p + 1) == '"') {
                        if (cap > 1) {
                            out[out_len++] = '"';
                            cap--;
                        }
                        p += 2;
                        has_content = true;
                    } else {
                        // End of quoted field
                        p++;
                        break;
                    }
                } else {
                    if (cap > 1) {
                        out[out_len++] = *p;
                        cap--;
                    }
                    p++;
                    has_content = true;
                }
            } else {
                if (*p == ',') {
                    p++;
                    break;
                }
                if (*p == '\0' || *p == '\n' || *p == '\r') {
                    break;
                }
                if (cap > 1) {
                    out[out_len++] = *p;
                    cap--;
                }
                p++;
                has_content = true;
            }
        }

        // If we finished parsing a field
        if (current_idx == idx) {
            out[out_len] = '\0';
            return (int)out_len;
        }

        // Skip delimiters and whitespace if not at end of string
        if (*p == ',') {
            p++;
            current_idx++;
        } else {
            // End of line or string
            if (current_idx != idx) {
                return -1;
            }
        }
    }
}