#include <string.h>
#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) {
        return -1;
    }

    const char *p = line;
    int field = 0;
    size_t out_len = 0;

    while (field <= idx) {
        // Skip leading whitespace (not required by RFC 4180, but common practice)
        while (*p == ' ' || *p == '\t') {
            p++;
        }

        if (*p == '\0') {
            return -1; // End of line reached before finding field
        }

        if (field == idx) {
            // Process the target field
            size_t remaining_cap = cap - 1; // Leave room for NUL
            size_t unescaped_len = 0;

            if (*p == '"') {
                // Quoted field
                p++; // Skip opening quote
                while (*p != '\0' && *p != '"') {
                    if (*p == '"' && *(p + 1) == '"') {
                        // Escaped quote
                        if (out_len < remaining_cap) {
                            out[out_len++] = '"';
                        }
                        unescaped_len++;
                        p += 2;
                    } else {
                        if (out_len < remaining_cap) {
                            out[out_len++] = *p;
                        }
                        unescaped_len++;
                        p++;
                    }
                }
                if (*p == '"') {
                    p++; // Skip closing quote
                }
            } else {
                // Unquoted field
                while (*p != '\0' && *p != ',') {
                    if (out_len < remaining_cap) {
                        out[out_len++] = *p;
                    }
                    unescaped_len++;
                    p++;
                }
            }

            // NUL-terminate
            out[out_len] = '\0';

            // Skip trailing whitespace after field (not required by RFC 4180)
            while (*p == ' ' || *p == '\t') {
                p++;
            }

            // Skip comma if present
            if (*p == ',') {
                p++;
            }

            return (int)unescaped_len;
        }

        // Skip current field
        if (*p == '"') {
            p++; // Skip opening quote
            while (*p != '\0' && *p != '"') {
                if (*p == '"' && *(p + 1) == '"') {
                    p += 2;
                } else {
                    p++;
                }
            }
            if (*p == '"') {
                p++; // Skip closing quote
            }
        } else {
            while (*p != '\0' && *p != ',') {
                p++;
            }
        }

        // Skip comma
        if (*p == ',') {
            p++;
        }

        field++;
    }

    return -1;
}