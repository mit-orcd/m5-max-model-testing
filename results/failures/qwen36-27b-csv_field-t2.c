```c
#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) {
        return -1;
    }

    const char *p = line;
    int i = 0;

    // Helper to skip to the next field start
    // Returns 0 if field exists, -1 if not (end of line reached before idx found)
    // Advances p to the start of the field content (after comma and optional quote)
    
    while (i < idx) {
        // Skip leading whitespace? RFC 4180 doesn't mandate trimming, but commas are delimiters.
        // A field starts after a comma or at the beginning of the line.
        
        // Find the start of the current field
        while (*p != '\0' && *p != ',' && *p != '"') {
            // If we hit end of line before finding a comma, the previous field was the last one.
            // But we are looking for the delimiter to move to the next field.
            // Actually, we need to parse the current field to skip it.
            // Let's restart the logic: we are at the start of a field.
            break;
        }

        // If we are at the end of the string, there are no more fields.
        if (*p == '\0') {
            return -1;
        }

        // We are at the start of a field (either at beginning of line or after a comma)
        // Check if it's a quoted field
        if (*p == '"') {
            p++; // skip opening quote
            while (*p != '\0') {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        p += 2; // skip doubled quote
                    } else if (*(p + 1) == ',') {
                        p += 2; // skip closing quote and comma
                        break;
                    } else if (*(p + 1) == '\0') {
                        // End of line after closing quote
                        p++; // skip closing quote
                        break;
                    } else {
                        // Invalid CSV: quote not followed by quote, comma, or end
                        // Treat as normal field? Or error? RFC says invalid.
                        // For robustness, let's just treat the quote as part of the field or stop.
                        // Let's stop parsing this field and move to next.
                        p++; 
                        break;
                    }
                } else {
                    p++;
                }
            }
        } else {
            // Unquoted field: end at comma or end of string
            while (*p != '\0' && *p != ',') {
                p++;
            }
            if (*p == ',') {
                p++; // skip comma
            }
        }

        // Now p is at the start of the next field (or end of string)
        i++;
        
        // If we reached the end of the string and we still need more fields
        if (*p == '\0' && i < idx) {
            return -1;
        }
    }

    // Now p is at the start of the target field (idx)
    // Check if we've reached the end of the line unexpectedly
    if (*p == '\0' && i < idx) {
        return -1;
    }
    // If i == idx, we are at the correct field. If *p is '\0', it's an empty field at the end.
    
    // Parse the field at idx
    size_t out_len = 0;
    
    if (*p == '"') {
        p++; // skip opening quote
        while (*p != '\0') {
            if (*p == '"') {
                if (*(p + 1) == '"') {
                    // Doubled quote
                    if (out_len < cap - 1) {
                        out[out_len++] = '"';
                    }
                    p += 2;
                } else if (*(p + 1) == ',') {
                    // End of quoted field
                    p += 2; // skip closing quote and comma
                    break;
                } else if (*(p + 1) == '\0') {
                    // End of line
                    p++; // skip closing quote
                    break;
                } else {
                    // Invalid, treat as end or escape? Let's treat as literal quote and move on?
                    // RFC says this is invalid. Let