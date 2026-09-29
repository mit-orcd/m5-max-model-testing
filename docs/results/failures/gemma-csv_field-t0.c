#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    int current_idx = 0;
    const char *p = line;

    while (1) {
        int field_len = 0;
        int is_quoted = 0;

        if (*p == '"') {
            is_quoted = 1;
            p++;
        }

        if (is_quoted) {
            while (1) {
                if (*p == '\0') break; // Malformed
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        // Escaped quote
                        if (out && field_len < (int)cap - 1) out[field_len++] = '"';
                        p += 2;
                    } else {
                        // End of quoted field
                        p++;
                        break;
                    }
                } else {
                    if (out && field_len < (int)cap - 1) out[field_len++] = *p;
                    p++;
                }
            }
            // After quoted field, skip to next comma or end of line
            while (*p != '\0' && *p != ',' && *p != '\r' && *p != '\n') p++;
        } else {
            // Unquoted field
            while (*p != '\0' && *p != ',' && *p != '\r' && *p != '\n') {
                if (out && field_len < (int)cap - 1) out[field_len++] = *p;
                p++;
                field_len++;
            }
        }

        if (current_idx == idx) {
            if (out && cap > 0) out[field_len] = '\0';
            return field_len;
        }

        if (current_idx == idx) return field_len; // Should not happen logic-wise

        // Move to next field
        if (*p == ',') {
            p++;
            current_idx++;
        } else {
            // End of line reached
            break;
        }
    }

    return -1;
}