#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (idx < 0 || !line || !out || cap == 0) {
        return -1;
    }

    int field = 0;
    const char *p = line;
    size_t out_len = 0;

    while (*p) {
        if (*p == '"') {
            // Quoted field
            if (field == idx) {
                p++; // Skip opening quote
                while (*p) {
                    if (*p == '"') {
                        if (p[1] == '"') {
                            // Escaped quote
                            if (out_len < cap - 1) {
                                out[out_len++] = '"';
                            }
                            p++; // Skip next quote
                        } else {
                            // End of quoted field
                            break;
                        }
                    } else {
                        if (out_len < cap - 1) {
                            out[out_len++] = *p;
                        }
                    }
                    p++;
                }
                out[out_len] = '\0';
                return (int)out_len;
            } else {
                // Skip to end of quoted field
                p++;
                while (*p) {
                    if (*p == '"') {
                        if (p[1] == '"') {
                            p++;
                        } else {
                            break;
                        }
                    }
                    p++;
                }
                if (*p) p++; // Skip closing quote
                if (*p == ',') p++; // Skip comma
            }
        } else {
            // Unquoted field
            if (field == idx) {
                while (*p && *p != ',') {
                    if (out_len < cap - 1) {
                        out[out_len++] = *p;
                    }
                    p++;
                }
                out[out_len] = '\0';
                return (int)out_len;
            } else {
                while (*p && *p != ',') p++;
                if (*p == ',') p++;
            }
        }
        field++;
        if (*p == ',') p++;
    }

    // Handle last field without trailing comma
    if (field == idx) {
        if (out_len < cap - 1) {
            out[out_len] = '\0';
        }
        return (int)out_len;
    }

    return -1;
}