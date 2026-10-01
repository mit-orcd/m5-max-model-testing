#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) {
        return -1;
    }

    const char *p = line;
    int current_idx = 0;

    while (*p != '\0') {
        if (current_idx == idx) {
            size_t out_len = 0;
            if (*p == '"') {
                // Quoted field
                p++; // skip opening quote
                while (*p != '\0') {
                    if (*p == '"') {
                        if (*(p + 1) == '"') {
                            // Escaped quote
                            if (out_len < cap - 1) {
                                out[out_len++] = '"';
                            }
                            p += 2;
                        } else {
                            // End of quoted field
                            p++; // skip closing quote
                            break;
                        }
                    } else {
                        if (out_len < cap - 1) {
                            out[out_len++] = *p;
                        }
                        p++;
                    }
                }
                out[out_len] = '\0';
                // Skip to next comma or end
                if (*p == ',') {
                    p++;
                }
                return (int)out_len;
            } else {
                // Unquoted field
                while (*p != '\0' && *p != ',') {
                    if (out_len < cap - 1) {
                        out[out_len++] = *p;
                    }
                    p++;
                }
                out[out_len] = '\0';
                if (*p == ',') {
                    p++;
                }
                return (int)out_len;
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
        } else {
            while (*p != '\0' && *p != ',') {
                p++;
            }
        }

        if (*p == ',') {
            p++;
        }
        current_idx++;
    }

    // If we've processed all fields and idx is still not reached, it's out of range
    // But we need to check if the last field was the one we wanted
    // The loop above increments current_idx after processing each field
    // If the line ended with a field (no trailing comma), current_idx was incremented after that field
    // So if idx == current_idx - 1, we should have caught it in the loop
    
    // Actually, let's reconsider: if the line is empty or we've gone past all fields
    if (idx < 0) {
        return -1;
    }
    
    // Check if we're at the end and the last field was the one we wanted
    // The loop processes fields and increments current_idx after each
    // If the line ends with a field, that field is processed and current_idx is incremented
    // So if idx == current_idx - 1, it was processed in the loop
    
    // If we get here, idx was not found
    return -1;
}