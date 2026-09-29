#include <stddef.h>
#include <stdbool.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || idx < 0) return -1;

    int current_idx = 0;
    const char *p = line;

    while (current_idx <= idx) {
        bool in_quotes = false;
        size_t written = 0;
        bool field_reached = false;

        // If we are looking for a specific index, we must skip previous fields
        if (current_idx < idx) {
            // Skip current field
            p = line;
            in_quotes = false;
            while (*p) {
                if (*p == '"') {
                    if (in_quotes && *(p + 1) == '"') {
                        p += 2; // Skip escaped quote
                    } else {
                        in_quotes = !in_quotes;
                        p++;
                    }
                } else if (*p == ',' && !in_quotes) {
                    p++;
                    break;
                } else {
                    p++;
                }
            }
            if (*p == '\0' && current_idx < idx) return -1;
            current_idx++;
            continue;
        }

        // We are at the target index
        // Note: p is already at the start of the field from the skip logic or initial call
        // But if current_idx == 0, p is line. If current_idx > 0, p is after the comma.
        
        // Re-sync p to start of field if we just skipped a comma
        // (The skip logic above moves p to the char after the comma)
        
        in_quotes = false;
        if (*p == '"') {
            in_quotes = true;
            p++;
        }

        while (*p) {
            if (in_quotes) {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        // Escaped quote
                        if (written < cap - 1) {
                            out[written++] = '"';
                        }
                        p += 2;
                    } else {
                        // End of quoted field
                        in_quotes = false;
                        p++;
                        // After closing quote, must be comma or end of string
                        while (*p && *p != ',' && *p != '\r' && *p != '\n') p++; 
                        break;
                    }
                } else {
                    if (written < cap - 1) {
                        out[written++] = *p;
                    }
                    p++;
                }
            } else {
                if (*p == ',' || *p == '\r' || *p == '\n' || *p == '\0') {
                    break;
                } else {
                    if (written < cap - 1) {
                        out[written++] = *p;
                    }
                    p++;
                }
            }
        }

        out[written] = '\0';
        return (int)written;
    }

    return -1;
}