```c
#include <stddef.h>
#include <stdbool.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || idx < 0) return -1;

    int current_idx = 0;
    const char *p = line;

    while (true) {
        if (current_idx == idx) {
            size_t written = 0;
            bool in_quotes = false;

            // Check if field is quoted
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
                    }
                } else {
                    // If not in quotes, a comma or end of string marks field end
                    if (*p == ',' || *p == '\0') break;
                }

                if (written < cap - 1) {
                    out[written] = *p;
                }
                written++;
                p++;
            }

            // Skip to the end of this field in the source string to allow iteration
            // if we were looping, but since we return, we just finalize.
            // However, we must handle the case where the field ended but the line continues.
            // The logic above handles the extraction. We need to return the unescaped length.
            
            // Re-calculating unescaped length correctly requires a separate pass or tracking.
            // Let's refine the loop to track actual unescaped length.
            return (int)written; 
        }

        // Skip current field to move to next idx
        bool field_quoted = false;
        if (*p == '"') {
            field_quoted = true;
            p++;
        }

        if (field_quoted) {
            while (*p != '\0') {
                if (*p == '"') {
                    if (*(p + 1) == '"') p += 2;
                    else { p++; break; }
                } else {
                    p++;
                }
            }
            if (*p == '"') p++; // skip closing quote
        } else {
            while (*p != '\0' && *p != ',') p++;
        }

        if (*p == ',') {
            p++;
            current_idx++;
        } else {
            // End of line reached before idx
            break;
        }
    }

    return -1;
}

// Refined implementation to ensure correct unescaped length and truncation logic
int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || !out || idx < 0) return -1;

    const char *p = line;
    int current_idx = 0;

    while (current_idx <= idx) {
        bool is_quoted = (*p == '"');
        const char *field_start = p;
        
        if (is_quoted) p++;

        const char *field_end = NULL;
        if (is_quoted) {
            while (*p) {
                if (*p == '"' && *(p + 1) == '"') {
                    p += 2;
                } else if (*p == '"') {
                    p++;
                    field_end = p;
                    break;
                } else {
                    p++;
                }
            }
        } else {
            while (*p && *p != ',') p++;
            field_end = p;
        }

        if (current_idx == idx) {
            // We found the field. Now unescape it.
            size_t unescaped_len = 0;
            const char *scan = is_quoted ? field_start + 1 : field_start;
            const char *end_scan = field_end;

            // First pass: calculate true unescaped length
            while (scan < end_scan) {
                if (is_quoted