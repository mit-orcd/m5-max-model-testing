#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) return -1;
    if (idx < 0) return -1;

    const char *p = line;
    int field_idx = 0;
    size_t out_len = 0;

    while (*p != '\0') {
        if (field_idx > idx) break;

        if (field_idx == idx) {
            if (*p == '"') {
                p++; // skip opening quote
                while (*p != '\0') {
                    if (*p == '"') {
                        if (p[1] == '"') { // escaped quote
                            if (out_len < cap - 1) out[out_len++] = '"';
                            p += 2;
                        } else { // closing quote
                            p++; // skip closing quote
                            break;
                        }
                    } else {
                        if (out_len < cap - 1) out[out_len++] = *p;
                        p++;
                    }
                }
            } else {
                while (*p != '\0' && *p != ',' && *p != '\n' && *p != '\r') {
                    if (out_len < cap - 1) out[out_len++] = *p;
                    p++;
                }
            }
            out[out_len] = '\0';
            return (int)out_len;
        }

        // Skip to end of current field
        if (*p == '"') {
            p++; // skip opening quote
            while (*p != '\0') {
                if (*p == '"') {
                    if (p[1] == '"') p += 2; // escaped quote
                    else { p++; break; }     // closing quote
                } else {
                    p++;
                }
            }
        } else {
            while (*p != '\0' && *p != ',' && *p != '\n' && *p != '\r') p++;
        }

        // Skip delimiter if present
        if (*p == ',') p++;
        else if (*p == '\n' || *p == '\r') {
            // Handle CRLF or LF
            if (*p == '\r' && p[1] == '\n') p += 2;
            else if (*p == '\n' || *p == '\r') p++;
            break; // end of line
        }

        field_idx++;
    }

    return -1; // idx out of range
}