#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) {
        return -1;
    }

    const char *p = line;
    int current_idx = 0;

    while (*p != '\0') {
        int is_quoted = 0;
        const char *field_start = p;
        const char *field_end = p;

        if (*p == '"') {
            is_quoted = 1;
            p++; // skip opening quote
            field_start = p;

            while (*p != '\0') {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        p += 2; // skip escaped quote
                    } else {
                        p++; // skip closing quote
                        break;
                    }
                } else {
                    p++;
                }
            }

            if (*p == ',') {
                p++; // skip comma
            } else if (*p == '\0') {
                // End of line
            } else {
                // Malformed CSV, but we treat it as end of field
            }
            field_end = p;
        } else {
            while (*p != ',' && *p != '\0') {
                p++;
            }
            field_end = p;
            if (*p == ',') {
                p++; // skip comma
            }
        }

        if (current_idx == idx) {
            // Extract the field
            size_t out_len = 0;
            const char *src = field_start;
            const char *end = field_end;

            if (is_quoted) {
                while (src < end) {
                    if (*src == '"' && *(src + 1) == '"' && src + 1 < end) {
                        if (out_len < cap - 1) {
                            out[out_len++] = '"';
                        }
                        src += 2;
                    } else {
                        if (out_len < cap - 1) {
                            out[out_len++] = *src;
                        }
                        src++;
                    }
                }
            } else {
                while (src < end) {
                    if (out_len < cap - 1) {
                        out[out_len++] = *src;
                    }
                    src++;
                }
            }

            out[out_len] = '\0';
            return (int)out_len;
        }

        current_idx++;

        if (*p == '\0') {
            break;
        }
    }

    // Check if idx is out of range
    if (idx < 0 || idx >= current_idx) {
        return -1;
    }

    return -1;
}