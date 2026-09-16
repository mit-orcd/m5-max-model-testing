#include <string.h>
#include <stdbool.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (idx < 0 || line == NULL || out == NULL) {
        return -1;
    }

    const char *ptr = line;
    int current = 0;
    size_t out_pos = 0;
    size_t i;
    bool in_quotes = false;

    while (*ptr != '\0' && current <= idx) {
        // Skip whitespace if the field is empty or quoted
        if (*ptr == ' ' || *ptr == '\t') {
            if (current == idx && !in_quotes) {
                ptr++;
                continue;
            }
        }

        if (*ptr == '"') {
            in_quotes = !in_quotes;
            ptr++;
            continue;
        }

        if (*ptr == ',' && !in_quotes) {
            current++;
            // Skip any whitespace after the delimiter
            while (*ptr == ' ' || *ptr == '\t') ptr++;
            ptr++;
            continue;
        }

        if (current == idx) {
            if (in_quotes) {
                if (*ptr == '"' && *(ptr + 1) == '"') {
                    // Unescape doubled quote
                    if (out_pos < cap - 1) {
                        out[out_pos++] = '"';
                    }
                    ptr += 2;
                } else {
                    if (out_pos < cap - 1) {
                        out[out_pos++] = *ptr;
                    }
                    ptr++;
                }
            } else {
                if (out_pos < cap - 1) {
                    out[out_pos++] = *ptr;
                }
                ptr++;
            }
        } else {
            ptr++;
        }
    }

    // Ensure null termination
    if (out_pos < cap) {
        out[out_pos] = '\0';
    } else if (cap > 0) {
        out[cap - 1] = '\0';
    }

    return (current == idx) ? (int)out_pos : -1;
}