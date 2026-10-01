#include <stddef.h>
#include <stdbool.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) return -1;

    int current_idx = 0;
    const char *p = line;

    while (true) {
        if (current_idx == idx) {
            size_t written = 0;
            bool in_quotes = false;

            if (*p == '"') {
                in_quotes = true;
                p++;
            }

            while (*p != '\0') {
                if (in_quotes) {
                    if (*p == '"') {
                        if (*(p + 1) == '"') {
                            // Escaped quote
                            if (written < cap - 1) {
                                out[written++] = '"';
                            }
                            p += 2;
                            continue;
                        } else {
                            // End of quoted field
                            in_quotes = false;
                            p++;
                            break;
                        }
                    } else {
                        if (written < cap - 1) {
                            out[written++] = *p;
                        }
                        p++;
                    }
                } else {
                    // This case handles if idx was reached but we aren't in quotes
                    // (shouldn't happen with correct logic, but for safety)
                    break;
                }
            }
            
            // If we were in quotes and hit end of string without closing quote, 
            // it's technically malformed, but we treat it as end of field.
            
            out[written] = '\0';
            return (int)written;
        }

        // Skip the current field to find the next comma
        bool in_quotes = false;
        while (*p != '\0') {
            if (*p == '"') {
                if (in_quotes && *(p + 1) == '"') {
                    p += 2; // Skip escaped quote
                    continue;
                }
                in_quotes = !in_quotes;
            } else if (*p == ',' && !in_quotes) {
                p++; // Move past comma
                break;
            }
            p++;
            if (*p == '\0') break;
        }

        if (current_idx == idx) return -1; // Should not be reachable
        
        if (*p == '\0') {
            // If we reached end of line and haven't found idx
            return -1;
        }

        current_idx++;
        if (*p == ',') p++; // Move to start of next field
        
        if (current_idx > idx) return -1;
    }
}