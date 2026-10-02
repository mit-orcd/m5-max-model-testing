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
                    // We are outside quotes, but this shouldn't happen 
                    // if the CSV is well-formed and we are in the target field.
                    break;
                }
            }
            
            // If we were in quotes and didn't find the closing quote, 
            // we treat the rest of the string as the field.
            
            out[written] = '\0';
            return (int)written;
        }

        // Skip the current field to move to the next one
        bool field_in_quotes = false;
        if (*p == '"') {
            field_in_quotes = true;
            p++;
        }

        while (*p != '\0') {
            if (field_in_quotes) {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        p += 2; // Skip escaped quote
                    } else {
                        field_in_quotes = false;
                        p++;
                        break;
                    }
                } else {
                    p++;
                }
            } else {
                if (*p == ',') {
                    break;
                }
                p++;
            }
        }

        if (*p == ',') {
            p++; // Skip the comma
            current_idx++;
        } else {
            // End of line reached
            break;
        }
    }

    return -1;
}