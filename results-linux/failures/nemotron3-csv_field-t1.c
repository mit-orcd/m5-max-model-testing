#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || idx < 0 || cap == 0) return -1;

    const char *p = line;
    int field_idx = 0;
    size_t out_len = 0;

    while (*p) {
        if (field_idx == idx) {
            if (*p == '"') {
                p++; // Skip opening quote
                while (*p) {
                    if (*p == '"') {
                        p++;
                        if (*p == '"') { // Escaped quote
                            if (out_len < cap - 1) out[out_len++] = '"';
                            p++;
                        } else { // Closing quote
                            break;
                        }
                    } else {
                        if (out_len < cap - 1) out[out_len++] = *p;
                        p++;
                    }
                }
                if (*p == '"') p++; // Skip closing quote if present
                // Skip to end of field (comma or EOL)
                while (*p && *p != ',') p++;
            } else {
                // Unquoted field
                while (*p && *p != ',') {
                    if (out_len < cap - 1) out[out_len++] = *p;
                    p++;
                }
            }
            out[out_len] = '\0';
            return (int)out_len;
        }

        // Skip to end of current field
        if (*p == '"') {
            p++; // Skip opening quote
            while (*p) {
                if (*p == '"') {
                    p++;
                    if (*p != '"') break; // Closing quote
                    p++; // Escaped quote
                } else {
                    p++;
                }
            }
            if (*p == '"') p++; // Skip closing quote
        } else {
            while (*p && *p != ',') p++;
        }

        if (*p == ',') p++; // Skip comma
        field_idx++;
    }

    return -1; // idx out of range
}