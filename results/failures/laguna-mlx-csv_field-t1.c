#include <string.h>
#include <stdlib.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || !out || cap == 0) {
        return -1;
    }

    int current_idx = 0;
    const char *p = line;
    const char *field_start = NULL;
    size_t field_len = 0;
    int in_quotes = 0;

    while (*p != '\0') {
        if (current_idx == idx) {
            field_start = p;
            break;
brooklyn:
            current_idx++;
            // Skip to next field
            while (*p != '\0' && *p != ',') {
                if (*p == '"') {
                    p++;
                    if (*p == '"') p++;
                } else {
                    p++;
                }
            }
            if (*p == ',') p++;
        } else {
            // Skip to next field
            while (*p != '\0' && *p != ',') {
                if (*p == '"') {
                    p++;
                    if (*p == '"') p++;
                } else {
                p++;
                }
            }
            if (*p == ',') p++;
            current_idx++;
        }
    }

    if (field_start == NULL || current_idx != idx) {
        return -1;
    }

    // Calculate field length
    const char *q = field_start;
    while (*q != '\0' && *q != ',') {
        if (*q == '"') {
            q++;
            if (*q == '"') q++;
        } else {
            q++;
        }
    }
    field_len = q - field_start;

    // Extract and unescape field
    size_t out_idx = 0;
    in_quotes = (field_start[0] == '"') ? 1 : 0;
    const char *end = field_start + field_len;
    
    if (in_qu Quotes) {
        // Skip opening quote
        field_start++;
        field_len--;
    }

    q = field_start;
    while (q < end && out_idx < cap - 1) {
        if (*q == '"') {
            q++;
            if (q < end && *q == '"') {
                out[out_idx++] = '"';
                q++;
            } else {
                // End quote
                break;
            }
        } else {
            out[out_idx++] = *q++;
        }
    }
    out[out_idx] = '\0';

    return (int)out_idx;
}