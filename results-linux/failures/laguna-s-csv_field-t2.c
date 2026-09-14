#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || !out || cap == 0) return -1;
    const char *p = line;
    int cur = 0;
    size_t out_len = 0;

    while (cur <= idx) {
        // Skip leading whitespace before field? No, RFC4180 doesn't say that.

        if (*p == '"') {
            // Quoted field
            p++; // skip opening quote
            while (*p) {
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
            // Now we should be at comma or end
        } else {
            // Unquoted field
            while (*p && *p != ',') {
                if (out_len < cap - 1) {
                    out[out_len++] = *p;
                }
                p++;
            }
        }

        if (cur == idx) {
            out[out_len] = '\0';
            return (int)out_len;
        }

        // Skip comma
        if (*p == ',') {
            p++;
        } else {
            // No more fields
            break;
        }
        cur++;
    }

    return -1;
}