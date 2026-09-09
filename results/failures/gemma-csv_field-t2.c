```c
#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    int current_idx = 0;
    const char *p = line;

    while (1) {
        int field_len = 0;
        int in_quotes = 0;
        int field_done = 0;

        if (*p == '"') {
            in_quotes = 1;
            p++;
        }

        while (*p != '\0') {
            if (in_quotes) {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        // Escaped quote
                        if (out && field_len < (int)cap - 1) out[field_len++] = '"';
                        p += 2;
                        continue;
                    } else {
                        // End of quoted field
                        p++;
                        in_quotes = 0;
                        break;
                    }
                }
                if (out && field_len < (int)cap - 1) out[field_len++] = *p;
                p++;
            } else {
                if (*p == '"') {
                    // This case handles malformed input where quote appears mid-field
                    if (out && field_len < (int)cap - 1) out[field_len++] = *p;
                    p++;
                } else if (*p == ',') {
                    // End of field
                    p++;
                    field_done = 1;
                    break;
                } else {
                    if (out && field_len < (int)cap - 1) out[field_len++] = *p;
                    p++;
                }
            }
        }

        if (current_idx == idx) {
            if (out) out[field_len] = '\0';
            return field_len;
        }

        if (current_idx == idx) return field_len; // Should not happen due to logic

        // If we reached end of string and it wasn't the target index
        if (*p == '\0' && !field_done && current_idx != idx) {
            // If we were in the middle of a field, it's the last field
            // If idx is higher than found fields, we return -1
            return -1;
        }

        current_idx++;
        if (current_idx > idx) return -1;
        
        // If we hit end of line and haven't reached idx
        if (*p == '\0' && !field_done) {
            // Check if we just finished the last field
            // If idx is still greater than current_idx, it's out of range
            // However, the loop logic needs to handle the final field
        }
        
        // If we are at the end of the string, break to return -1
        if (*p == '\0' && !field_done) break;
    }

    return -1;
}

// Redefining logic for cleaner single-pass approach
int csv_field(const char *line, int idx, char *out, size_t cap) {
    int current_idx = 0;
    const char *p = line;

    while (1) {
        int field_len = 0;
        int in_quotes = 0;
        int field_finished = 0;

        // Determine if field starts with quote
        int starts_with_quote = (p != NULL && *p == '"');
        if (starts_with_quote) {
            in_quotes = 1;
            p++;
        }

        // Process field
        while (p != NULL && *p != '\0') {
            if (in_quotes) {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        if (out && field_len < (int)cap - 1) out[field_len++] = '"';
                        p += 2;
                        continue;
                    } else {
                        p++;
                        in_quotes = 0;
                        // After quoted field, we expect comma or end of string
                        break;
                    }
                }
                if (out && field_len < (int)cap - 1) out[field_len++] = *p;
                p++;
            } else {
                if (*p == ',') {