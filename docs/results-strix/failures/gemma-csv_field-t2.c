#include <stddef.h>
#include <stdbool.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || idx < 0) return -1;

    int current_idx = 0;
    const char *p = line;

    while (current_idx <= idx) {
        bool in_quotes = false;
        size_t written = 0;
        bool field_finished = false;

        // If we are looking for a specific index, we skip preceding fields
        if (current_idx < idx) {
            in_quotes = false;
            while (*p != '\0') {
                if (!in_quotes) {
                    if (*p == '"') in_quotes = true;
                    else if (*p == ',' || *p == '\n' || *p == '\r') break;
                } else {
                    if (*p == '"') {
                        if (*(p + 1) == '"') p++; // Skip escaped quote
                        else in_quotes = false;
                    } else if (*p == ',' || *p == '\n' || *p == '\r') {
                        // This is a logic error in CSV structure, but we treat it as end of field
                        break;
                    }
                }
                p++;
            }
            // Move past the delimiter of the skipped field
            while (*p != '\0' && *p != ',' && *p != '\n' && *p != '\r') p++;
            if (*p == ',') p++;
            else if (*p == '\0') return -1; // End of line reached before idx
            
            current_idx++;
            continue;
        }

        // Extract the target field
        if (*p == '"') {
            in_quotes = true;
            p++; // Skip opening quote
        } else {
            in_quotes = false;
        }

        while (*p != '\0') {
            if (in_quotes) {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        // Escaped quote
                        if (written < cap - 1) out[written++] = '"';
                        p += 2;
                        continue;
                    } else {
                        // Closing quote
                        in_quotes = false;
                        p++;
                        // After closing quote, must be delimiter or end of line
                        while (*p != '\0' && *p != ',' && *p != '\n' && *p != '\r') p++;
                        field_finished = true;
                        break;
                    }
                }
            } else {
                if (*p == ',' || *p == '\n' || *p == '\r') {
                    field_finished = true;
                    break;
                }
            }

            if (written < cap - 1) {
                out[written++] = *p;
            }
            p++;
        }

        if (field_finished || *p == '\0') {
            out[written] = '\0';
            // If we finished at a delimiter, skip it for next potential calls (though not used here)
            if (*p == ',') p++; 
            return (int)written;
        }

        // If we exited loop without field_finished and p is at end, idx was out of range
        return -1;
    }

    return -1;
}