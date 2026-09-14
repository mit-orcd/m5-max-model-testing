#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || !out || cap == 0) return -1;

    size_t len = 0;
    int current_idx = 0;
    int in_quotes = 0;
    const char *p = line;

    // Skip leading whitespace if not in quotes (RFC 4180 doesn't strictly define whitespace skipping,
    // but standard CSV parsers often treat it as part of the field or ignore it. 
    // Here we treat whitespace as part of the field unless it's a delimiter context).
    // Actually, RFC 4180 says fields are separated by commas. We just parse strictly.

    while (*p) {
        if (current_idx == idx) {
            // We are inside the target field
            if (*p == '"') {
                if (in_quotes) {
                    // Check for escaped quote ""
                    if (p[1] == '"') {
                        if (len < cap - 1) {
                            out[len++] = '"';
                        }
                        p += 2;
                        continue;
                    } else {
                        // End of quoted field
                        in_quotes = 0;
                        p++;
                        continue;
                    }
                } else {
                    // Start of quoted field
                    in_quotes = 1;
                    p++;
                    continue;
                }
            } else if (*p == ',') {
                // End of field (delimiter)
                break;
            } else {
                // Regular character
                if (len < cap - 1) {
                    out[len++] = *p;
                }
                p++;
            }
        } else {
            // We are before the target field, skip it
            if (*p == '"') {
                in_quotes = 1;
                p++;
                while (*p) {
                    if (*p == '"') {
                        if (p[1] == '"') {
                            p += 2;
                        } else {
                            in_quotes = 0;
                            p++;
                            break;
                        }
                    } else {
                        p++;
                    }
                }
            } else if (*p == ',') {
                current_idx++;
                p++;
                if (current_idx == idx) break;
            } else {
                p++;
            }
        }
    }

    if (current_idx != idx) {
        return -1;
    }

    out[len] = '\0';
    return (int)len;
}