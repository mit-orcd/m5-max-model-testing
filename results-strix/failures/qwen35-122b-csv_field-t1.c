#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || !out || cap == 0) {
        return -1;
    }

    int current_idx = 0;
    size_t out_len = 0;
    const char *p = line;

    while (*p) {
        if (current_idx == idx) {
            // We are at the target field
            int quoted = 0;
            if (*p == '"') {
                quoted = 1;
                p++;
            }

            while (*p) {
                if (quoted) {
                    if (*p == '"') {
                        if (*(p + 1) == '"') {
                            // Escaped quote
                            if (out_len + 1 < cap) {
                                out[out_len++] = '"';
                            }
                            p += 2;
                        } else {
                            // End of quoted field
                            p++;
                            break;
                        }
                    } else {
                        if (out_len + 1 < cap) {
                            out[out_len++] = *p;
                        }
                        p++;
                    }
                } else {
                    if (*p == ',') {
                        break;
                    }
                    if (out_len + 1 < cap) {
                        out[out_len++] = *p;
                    }
                    p++;
                }
            }

            out[out_len] = '\0';
            return (int)out_len;
        }

        // Skip to next field
        int quoted = 0;
        if (*p == '"') {
            quoted = 1;
            p++;
        }

        while (*p) {
            if (quoted) {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        p += 2;
                    } else {
                        p++;
                        break;
                    }
                } else {
                    p++;
                }
            } else {
                if (*p == ',') {
                    p++;
                    break;
                }
                p++;
            }
        }
        current_idx++;
    }

    return -1;
}