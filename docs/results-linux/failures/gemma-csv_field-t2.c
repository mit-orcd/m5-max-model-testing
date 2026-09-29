#include <stddef.h>
#include <stdbool.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) return -1;

    int current_idx = 0;
    const char *p = line;

    while (true) {
        bool in_quotes = false;
        size_t written = 0;
        bool field_reached = false;

        // Check if we are at the start of the target field
        if (current_idx == idx) {
            // If it's the first field, we start at p. 
            // If not, we must have skipped previous fields.
        }

        // Logic to skip or process fields
        // Since we need to handle quotes, we must parse character by character
        // to correctly identify field boundaries.
        
        // This loop handles the current field starting at 'p'
        const char *field_start = p;
        bool is_quoted = (field_start[0] == '"');
        
        if (is_quoted) {
            p++; // Skip opening quote
            while (*p != '\0') {
                if (*p == '"') {
                    if (p[1] == '"') { // Escaped quote
                        if (written < cap - 1) out[written++] = '"';
                        p += 2;
                    } else { // Closing quote
                        p++;
                        break;
                    }
                } else {
                    if (written < cap - 1) out[written++] = *p;
                    p++;
                }
                // If we hit a comma after closing quote, it's the end of field
                if (p > field_start && *(p-1) != '"' && *p == ',') break; 
            }
            // After closing quote, skip until comma or end of string
            while (*p != '\0' && *p != ',') p++;
        } else {
            while (*p != '\0' && *p != ',') {
                if (written < cap - 1) out[written++] = *p;
                p++;
            }
        }

        if (current_idx == idx) {
            out[written] = '\0';
            return (int)written;
        }

        // Move to next field
        if (*p == ',') {
            p++;
            current_idx++;
        } else {
            // End of line reached and we haven't found idx
            break;
        }
    }

    return -1;
}