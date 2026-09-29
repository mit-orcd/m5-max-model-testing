#include <stdio.h>
#include <string.h>
#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) {
        return -1;
    }

    const char *p = line;
    int field = 0;

    while (field < idx) {
        // Skip to end of current field
        if (*p == '"') {
            // Quoted field: scan until closing quote
            p++; // skip opening quote
            while (*p != '\0' && *p != '"') {
                if (*p == '"' && *(p + 1) == '"') {
                    p += 2; // skip doubled quote
                } else {
                    p++;
                }
            }
            if (*p == '"') {
                p++; // skip closing quote
            }
        } else {
            // Unquoted field: scan until comma or newline or end
            while (*p != '\0' && *p != ',' && *p != '\r' && *p != '\n') {
                p++;
            }
        }

        if (*p == '\0') {
            return -1; // idx out of range
        }

        // Move past comma
        if (*p == ',') {
            p++;
        }
        field++;
    }

    // Now p points to start of target field
    size_t len = 0;
    if (*p == '"') {
        // Quoted field
        p++; // skip opening quote
        while (*p != '\0' && *p != '"') {
            if (*p == '"' && *(p + 1) == '"') {
                if (len + 1 < cap) {
                    out[len] = '"';
                }
                len++;
                p += 2;
            } else {
                if (len + 1 < cap) {
                    out[len] = *p;
                }
                len++;
                p++;
            }
        }
        if (*p == '"') {
            p++; // skip closing quote
        }
    } else {
        // Unquoted field
        while (*p != '\0' && *p != ',' && *p != '\r' && *p != '\n') {
            if (len + 1 < cap) {
                out[len] = *p;
            }
            len++;
            p++;
        }
    }

    // Null-terminate
    if (len < cap) {
        out[len] = '\0';
    } else {
        out[cap - 1] = '\0';
    }

    return (int)len;
}