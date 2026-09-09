```c
#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || !out || cap == 0) return -1;

    const char *p = line;
    int current_idx = 0;
    int field_start = 0;
    int in_quotes = 0;
    int i = 0;

    // Skip to the start of the requested field
    while (current_idx < idx) {
        if (*p == '\0') return -1; // End of line before reaching idx

        if (in_quotes) {
            if (*p == '"') {
                if (*(p + 1) == '"') {
                    p += 2; // Skip escaped quote
                    continue;
                } else {
                    in_quotes = 0;
                    p++;
                    continue;
                }
            }
            p++;
            continue;
        }

        if (*p == '"') {
            in_quotes = 1;
            p++;
            continue;
        }

        if (*p == ',') {
            current_idx++;
            field_start = 0; // Reset for next field
            p++;
            continue;
        }

        p++;
    }

    // Now p points to the start of the requested field (or the comma after it if idx is out of bounds)
    // We need to find the actual start of the field content
    // Re-scan to find the start of the current field
    p = line;
    current_idx = 0;
    in_quotes = 0;
    
    while (current_idx < idx) {
        if (*p == '\0') return -1;
        
        if (in_quotes) {
            if (*p == '"') {
                if (*(p + 1) == '"') {
                    p += 2;
                    continue;
                } else {
                    in_quotes = 0;
                    p++;
                    continue;
                }
            }
            p++;
            continue;
        }

        if (*p == '"') {
            in_quotes = 1;
            p++;
            continue;
        }

        if (*p == ',') {
            current_idx++;
            p++;
            continue;
        }

        p++;
    }

    // p is now at the start of the field
    const char *field_start_ptr = p;
    const char *field_end_ptr = p;
    
    // Scan to find the end of the field
    while (*field_end_ptr != '\0' && *field_end_ptr != ',') {
        if (in_quotes) {
            if (*field_end_ptr == '"') {
                if (*(field_end_ptr + 1) == '"') {
                    field_end_ptr += 2;
                    continue;
                } else {
                    in_quotes = 0;
                    field_end_ptr++;
                    continue;
                }
            }
            field_end_ptr++;
            continue;
        }

        if (*field_end_ptr == '"') {
            in_quotes = 1;
            field_end_ptr++;
            continue;
        }

        field_end_ptr++;
    }

    // Calculate unescaped length
    size_t len = 0;
    const char *scan = field_start_ptr;
    while (scan < field_end_ptr) {
        if (in_quotes) {
            if (*scan == '"') {
                if (*(scan + 1) == '"') {
                    scan += 2;
                    continue;
                } else {
                    in_quotes = 0;
                    scan++;
                    continue;
                }
            }
            scan++;
            continue;
        }

        if (*scan == '"') {
            in_quotes = 1;
            scan++;
            continue;
        }

        len++;
        scan++;
    }

    if (len >= cap) {
        len = cap - 1;
    }

    // Copy and unescape
    size_t write_idx = 0;
    scan = field_start_ptr;
    in_quotes = 0;
    
    // Re-scan to copy
    while (scan < field_end_ptr && write_idx < len) {
        if (in_quotes) {
            if (*scan == '"') {
                if (*(scan + 1) == '"') {
                    out[write_idx++] = '"';
                    scan += 2;
                    continue;
                } else {
                    in_quotes = 0;