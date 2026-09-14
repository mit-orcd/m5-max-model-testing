#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) {
        return -1;
    }

    const char *p = line;
    int current_idx = 0;

    while (1) {
        // Skip leading whitespace? RFC-4180 doesn't mandate it, but standard parsing ignores it outside quotes.
        // Actually, strict RFC-4180: fields are comma-separated. Whitespace is part of the field unless quoted.
        // However, typically we just look for the start of the field.
        
        // Check if we are at the end of the line
        if (*p == '\0') {
            // If we are looking for the current field and we are at the end, it's an empty field at the end?
            // No, if we hit \0, there are no more fields.
            if (current_idx == idx) {
                // This case happens if the line ended exactly after a comma? 
                // e.g., "a," -> field 0 is "a", field 1 is "".
                // If p is at '\0', and we haven't skipped a comma to get here, it means we are processing the last field.
                // But the loop structure below handles field extraction.
                // Let's refine the loop.
            }
            return -1;
        }

        // Check if we need to skip a comma to get to the next field
        if (current_idx < idx) {
            if (*p == ',') {
                p++;
                current_idx++;
                continue;
            }
            // Find the end of the current field to skip it
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
                    return -1; // Malformed quoted field
                }
                // Now p is at the closing quote
                p++; // skip closing quote
                if (*p == ',') {
                    p++; // skip comma
                } else if (*p != '\0') {
                    return -1; // Expected comma or end after quoted field
                }
                current_idx++;
                continue;
            } else {
                // Unquoted field
                while (*p != '\0' && *p != ',') {
                    p++;
                }
                if (*p == ',') {
                    p++; // skip comma
                }
                current_idx++;
                continue;
            }
        }

        // We are at the start of the target field
        if (*p == '"') {
            // Quoted field
            p++; // skip opening quote
            size_t out_idx = 0;
            while (*p != '\0') {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        // Escaped quote
                        if (out_idx < cap - 1) {
                            out[out_idx++] = '"';
                        }
                        p += 2;
                    } else {
                        // Closing quote
                        p++;
                        break;
                    }
                } else {
                    if (out_idx < cap - 1) {
                        out[out_idx++] = *p;
                    }
                    p++;
                }
            }
            if (*p != '"') {
                return -1; // Malformed quoted field
            }
            out[out_idx] = '\0';
            // Skip trailing comma if present
            if (*p == ',') {
                p++;
            }
            return (int)out_idx;
        } else {
            // Unquoted field
            size_t out_idx = 0;
            while (*p != '\0' && *p != ',') {
                if (out_idx < cap - 1) {
                    out[out_idx++] = *p;
                }
                p++;
            }
            out[out_idx] = '\0';
            return (int)out_idx;
        }
    }
}