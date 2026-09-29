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

        // Check if we are at the start of a new field
        if (current_idx == 0) {
            in_quotes = (*p == '"');
            if (in_quotes) p++;
        } else {
            // Skip the delimiter (comma) from the previous field
            // Note: the loop logic below handles advancing p
        }

        if (current_idx == idx) {
            // We are parsing the target field
            while (*p != '\0') {
                if (in_quotes) {
                    if (*p == '"') {
                        if (*(p + 1) == '"') {
                            // Escaped quote: "" -> "
                            if (written < cap - 1) out[written++] = '"';
                            p += 2;
                        } else {
                            // End of quoted section
                            in_quotes = false;
                            p++;
                        }
                    } else {
                        if (written < cap - 1) out[written++] = *p;
                        p++;
                    }
                } else {
                    if (*p == ',') {
                        field_reached = true;
                        break;
                    } else if (*p == '"') {
                        // RFC 4180: quotes only allowed at start/end of field
                        // but we follow basic unescaping logic
                        in_quotes = true;
                        p++;
                    } else {
                        if (written < cap - 1) out[written++] = *p;
                        p++;
                    }
                }
            }
            
            // If we didn't hit a comma or end of string, it's technically invalid 
            // but we return what we have.
            if (written < cap) out[written] = '\0';
            else out[cap - 1] = '\0';
            
            return (int)written;
        } else {
            // We are skipping fields to reach idx
            in_quotes = (*p == '"');
            if (in_quotes) p++;

            while (*p != '\0') {
                if (in_quotes) {
                    if (*p == '"' && *(p + 1) == '"') p += 2;
                    else if (*p == '"') { in_quotes = false; p++; }
                    else p++;
                } else {
                    if (*p == ',') {
                        p++; // skip comma
                        break;
                    } else {
                        p++;
                    }
                }
            }
            
            if (*p == '\0' && current_idx < idx) return -1;
            current_idx++;
        }
    }

    return -1;
}