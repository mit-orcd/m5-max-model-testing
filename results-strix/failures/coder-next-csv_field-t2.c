#include <stddef.h>
#include <string.h>
#include <stdbool.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) {
        return -1;
    }

    const char *p = line;
    int field_num = 0;

    while (field_num < idx) {
        if (*p == '\0') {
            return -1;
        }
        bool in_quotes = false;
        while (*p != '\0') {
            if (!in_quotes && *p == ',') {
                p++;
                break;
            } else if (!in_quotes && *p == '"') {
                in_quotes = true;
            } else if (in_quotes && *p == '"') {
                if (*(p + 1) == '"') {
                    p++; // Skip the escaped quote
                } else {
                    in_quotes = false;
                }
            }
            p++;
        }
        field_num++;
    }

    // Now extract the target field
    if (*p == '\0') {
        return -1;
    }

    size_t written = 0;
    bool in_quotes = false;

    while (*p != '\0') {
        if (!in_quotes && *p == ',') {
            p++; // Skip the comma
            break;
        } else if (!in_quotes && *p == '"') {
            in_quotes = true;
        } else if (in_quotes && *p == '"') {
            if (*(p + 1) == '"') {
                // Escaped quote: add one quote to output
                if (written + 1 < cap) {
                    out[written++] = '"';
                }
                p += 2; // Skip both quotes
            } else {
                // End of quoted field
                in_quotes = false;
                p++;
            }
        } else {
            // Regular character
            if (written + 1 < cap) {
                out[written++] = *p;
            }
            p++;
        }
    }

    out[written] = '\0';
    return (int)written;
}