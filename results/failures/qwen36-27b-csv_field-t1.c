#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) {
        return -1;
    }

    const char *p = line;
    int current_idx = 0;
    int found = 0;

    while (*p != '\0') {
        if (current_idx == idx) {
            found = 1;
            break;
        }

        // Skip current field
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
            if (*p == ',') {
                p++; // skip comma
            }
        } else {
            // Unquoted field
            while (*p != ',' && *p != '\0') {
                p++;
            }
            if (*p == ',') {
                p++; // skip comma
            }
        }

        current_idx++;
    }

    if (!found) {
        return -1;
    }

    // Now p points to the start of the desired field
    size_t out_len = 0;
    int is_quoted = (*p == '"');

    if (is_quoted) {
        p++; // skip opening quote
        while (*p != '\0') {
            if (*p == '"') {
                if (*(p + 1) == '"') {
                    if (out_len + 1 < cap) {
                        out[out_len++] = '"';
                    }
                    p += 2;
                } else {
                    // Closing quote
                    p++; // skip closing quote
                    break;
                }
            } else {
                if (out_len + 1 < cap) {
                    out[out_len++] = *p;
                }
                p++;
            }
        }
        // Skip trailing comma if present
        if (*p == ',') {
            p++;
        }
    } else {
        while (*p != ',' && *p != '\0') {
            if (out_len + 1 < cap) {
                out[out_len++] = *p;
            }
            p++;
        }
        if (*p == ',') {
            p++;
        }
    }

    out[out_len] = '\0';
    return (int)out_len;
}