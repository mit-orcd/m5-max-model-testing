#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) {
        return -1;
    }

    const char *p = line;
    int field_count = 0;
    size_t out_len = 0;

    while (field_count <= idx && *p != '\0') {
        // Skip leading whitespace (not allowed in RFC-4180, but handle gracefully)
        while (*p == ' ' || *p == '\t') p++;

        if (*p == '"') {
            // Quoted field
            p++; // Skip opening quote
            out_len = 0;
            while (*p != '\0' && out_len < cap - 1) {
                if (*p == '"') {
                    if (p[1] == '"') {
                        // Escaped quote
                        out[out_len++] = '"';
                        p += 2;
                    } else {
                        // End of quoted field
                        p++; // Skip closing quote
                        break;
                    }
                } else {
                    out[out_len++] = *p++;
                }
            }
            // Skip to comma or end
            while (*p != ',' && *p != '\0' && *p != '\r' && *p != '\n') p++;
        } else {
            // Unquoted field
            out_len = 0;
            while (*p != ',' && *p != '\0' && *p != '\r' && *p != '\n' && out_len < cap - 1) {
                out[out_len++] = *p++;
            }
        }

        // Null-terminate if we have space
        if (out_len < cap) {
            out[out_len] = '\0';
        }

        if (field_count == idx) {
            return (int)out_len;
        }

        // Move past comma
        if (*p == ',') p++;

        field_count++;
    }

    // Check for trailing \r\n
    if (*p == '\r' && p[1] == '\n') p += 2;
    else if (*p == '\r' || *p == '\n') p++;

    return -1;
}