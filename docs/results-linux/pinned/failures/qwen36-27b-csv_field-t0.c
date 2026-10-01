#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) {
        return -1;
    }

    const char *p = line;
    int current_idx = 0;

    while (*p != '\0') {
        if (current_idx == idx) {
            // Start extracting the field
            size_t out_pos = 0;
            if (*p == '"') {
                // Quoted field
                p++; // skip opening quote
                while (*p != '\0') {
                    if (*p == '"') {
                        if (*(p + 1) == '"') {
                            // Escaped quote
                            if (out_pos < cap - 1) {
                                out[out_pos++] = '"';
                            }
                            p += 2;
                        } else {
                            // End of quoted field
                            p++; // skip closing quote
                            break;
                        }
                    } else {
                        if (out_pos < cap - 1) {
                            out[out_pos++] = *p;
                        }
                        p++;
                    }
                }
                // After closing quote, expect comma or end of string
                if (*p == ',') {
                    p++;
                }
                out[out_pos] = '\0';
                return (int)out_pos;
            } else {
                // Unquoted field
                while (*p != '\0' && *p != ',') {
                    if (out_pos < cap - 1) {
                        out[out_pos++] = *p;
                    }
                    p++;
                }
                if (*p == ',') {
                    p++;
                }
                out[out_pos] = '\0';
                return (int)out_pos;
            }
        }

        // Skip to next field
        if (*p == '"') {
            p++; // skip opening quote
            while (*p != '\0') {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        p += 2;
                    } else {
                        p++; // skip closing quote
                        break;
                    }
                } else {
                    p++;
                }
            }
            // After closing quote, expect comma or end of string
            if (*p == ',') {
                p++;
            }
        } else {
            while (*p != '\0' && *p != ',') {
                p++;
            }
            if (*p == ',') {
                p++;
            }
        }
        current_idx++;
    }

    // If we've processed all fields and idx is still not reached, it's out of range
    // But we need to check if the last field was the one we wanted
    // The loop above increments current_idx after processing each field.
    // If the line ends with a field (no trailing comma), current_idx will be one more than the last field index.
    // So if idx == current_idx - 1 and we haven't returned yet, we missed it? 
    // Actually, the logic above handles extraction when current_idx == idx at the start of the field.
    // If we exit the loop, it means we ran out of characters. 
    // Let's reconsider: if idx is beyond the number of fields, return -1.
    
    // Count fields to determine if idx is valid
    // Actually, the above loop structure: when we start processing a field, current_idx is the index of that field.
    // If we finish the line and never hit current_idx == idx, then idx is out of range.
    
    return -1;
}