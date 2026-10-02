#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) {
        return -1;
    }

    const char *p = line;
    int current_idx = 0;

    // Skip to the desired field index
    while (current_idx < idx) {
        if (*p == '\0') {
            return -1; // idx out of range
        }

        if (*p == '"') {
            // Quoted field
            p++; // skip opening quote
            while (*p != '\0') {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        p += 2; // escaped quote
                    } else {
                        p++; // closing quote
                        break;
                    }
                } else {
                    p++;
                }
            }
            if (*p != '"') {
                return -1; // malformed quoted field
            }
            // Skip comma after closing quote if present
            if (*(p + 1) == ',') {
                p += 2;
            } else if (*(p + 1) == '\0') {
                p++;
            } else {
                return -1; // malformed CSV
            }
        } else {
            // Unquoted field
            while (*p != '\0' && *p != ',') {
                p++;
            }
            if (*p == ',') {
                p++;
            } else if (*p == '\0') {
                p++;
            }
        }
        current_idx++;
    }

    // Now p points to the start of the field at idx
    // Check if we've reached end of string
    if (*p == '\0') {
        return -1; // idx out of range
    }

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
                    // Closing quote
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
        if (*p != '"') {
            return -1; // malformed quoted field
        }
        // After closing quote, expect comma or end of string
        if (*p == ',') {
            // valid, field ends here
        } else if (*p != '\0') {
            return -1; // malformed CSV
        }
    } else {
        // Unquoted field
        while (*p != '\0' && *p != ',') {
            if (out_len < cap - 1) {
                out[out_len++] = *p;
            }
            p++;
        }
    }

    out[out_len] = '\0';
    return (int)out_len;
}