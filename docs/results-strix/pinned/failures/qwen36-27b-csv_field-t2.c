#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) {
        return -1;
    }

    const char *p = line;
    int current_idx = 0;

    while (*p != '\0') {
        if (current_idx == idx) {
            size_t out_len = 0;
            if (*p == '"') {
                // Quoted field
                p++; // skip opening quote
                while (*p != '\0') {
                    if (*p == '"') {
                        if (*(p + 1) == '"') {
                            // Escaped quote
                            if (out_len < cap - 1) {
                                out[out_len++] = '"';
                            }
                            p += 2;
                        } else {
                            // End of quoted field
                            p++; // skip closing quote
                            break;
                        }
                    } else {
                        if (out_len < cap - 1) {
                            out[out_len++] = *p;
                        }
                        p++;
                    }
                }
                out[out_len] = '\0';
                // Skip to next comma or end
                if (*p == ',') {
                    p++;
                }
                return (int)out_len;
            } else {
                // Unquoted field
                while (*p != '\0' && *p != ',') {
                    if (out_len < cap - 1) {
                        out[out_len++] = *p;
                    }
                    p++;
                }
                out[out_len] = '\0';
                if (*p == ',') {
                    p++;
                }
                return (int)out_len;
            }
        }

        // Skip to next field
        if (*p == '"') {
            p++; // skip opening quote
            while (*p != '\0') {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        p += 2;
                    } else {
                        p++; // skip closing quote
                        break;
                    }
                } else {
                    p++;
                }
            }
        } else {
            while (*p != '\0' && *p != ',') {
                p++;
            }
        }

        if (*p == ',') {
            p++;
        }
        current_idx++;
    }

    // If we reached here, idx was out of range
    return -1;
}