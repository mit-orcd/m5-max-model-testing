#include <string.h>
#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (idx < 0 || cap == 0) return -1;

    const char *p = line;
    int current = 0;
    size_t out_len = 0;

    while (*p != '\0' && current <= idx) {
        if (*p == '"') {
            // Quoted field
            if (current != idx) {
                // Skip to closing quote
                p++;
                while (*p != '\0') {
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
                }
                // Skip comma after quoted field
                if (*p == ',') p++;
                current++;
            } else {
                // This is the field we want
                p++;
                while (*p != '\0') {
                    if (*p == '"') {
                        if (*(p + 1) == '"') {
                            if (out_len < cap - 1) {
                                out[out_len++] = '"';
                            }
                            p += 2;
                        } else {
                            p++;
                            break;
                        }
                    } else {
                        if (out_len < cap - 1) {
                            out[out_len++] = *p;
                        }
                        p++;
                    }
                }
                // Skip comma after quoted field
                if (*p == ',') p++;
                current++;
                break;
            }
        } else {
            // Unquoted field
            if (current == idx) {
                while (*p != '\0' && *p != ',') {
                    if (out_len < cap - 1) {
                        out[out_len++] = *p;
                    }
                    p++;
                }
                // Skip comma
                if (*p == ',') p++;
                current++;
                break;
            } else {
                // Skip until comma or end
                while (*p != '\0' && *p != ',') {
                    p++;
                }
                // Skip comma
                if (*p == ',') p++;
                current++;
            }
        }
    }

    out[out_len] = '\0';

    if (current <= idx) {
        // Field not found
        return -1;
    }

    return (int)out_len;
}