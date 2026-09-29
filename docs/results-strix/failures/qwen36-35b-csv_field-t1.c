#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (cap == 0) return -1;

    const char *p = line;
    int current_idx = 0;

    while (*p != '\0' && current_idx < idx) {
        if (*p == '"') {
            // Quoted field: skip until closing quote
            p++;
            while (*p != '\0') {
                if (*p == '"') {
                    p++;
                    if (*p == '"') {
                        // Escaped quote, skip both
                        p++;
                    } else {
                        // End of quoted field
                        break;
                    }
                } else {
                    p++;
                }
            }
            // Skip comma after field if present
            if (*p == ',') p++;
        } else {
            // Unquoted field: skip until comma or end
            while (*p != '\0' && *p != ',') p++;
            if (*p == ',') p++;
        }
        current_idx++;
    }

    if (*p == '\0' && current_idx != idx) {
        return -1;
    }

    // Now p points to the start of the desired field
    int len = 0;
    size_t i = 0;

    if (*p == '"') {
        // Quoted field
        p++;
        while (*p != '\0' && i < cap - 1) {
            if (*p == '"') {
                p++;
                if (*p == '"') {
                    out[i++] = '"';
                    p++;
                } else {
                    // End of quoted field
                    break;
                }
            } else {
                out[i++] = *p++;
            }
        }
        // Skip remaining part of quoted field if any (e.g., trailing content before comma)
        while (*p != '\0' && *p != ',') p++;
    } else {
        // Unquoted field
        while (*p != '\0' && *p != ',' && i < cap - 1) {
            out[i++] = *p++;
        }
    }

    out[i] = '\0';
    return len;
}