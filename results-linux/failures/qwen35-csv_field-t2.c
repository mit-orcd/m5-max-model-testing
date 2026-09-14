```c
#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL) return -1;
    
    const char *p = line;
    int current_idx = 0;
    
    // Handle empty line or just whitespace (treat as 0 fields if empty)
    // If line is empty, only idx 0 might return empty string if we consider it 1 field, 
    // but standard CSV parsers often treat empty line as 0 fields or 1 empty field.
    // Let's assume a line with content or empty string yields at least one field if idx==0.
    
    while (1) {
        if (current_idx == idx) {
            size_t len = 0;
            const char *field_start = p;
            int in_quotes = 0;
            
            // Check if field is quoted
            if (*p == '"') {
                in_quotes = 1;
                p++;
                while (*p) {
                    if (*p == '"') {
                        if (*(p + 1) == '"') {
                            // Escaped quote
                            p += 2;
                        } else {
                            // End of quoted field
                            p++;
                            break;
                        }
                    } else {
                        p++;
                    }
                }
                // Skip trailing comma if not at end of line
                if (*p == ',') {
                    p++;
                } else if (*p != '\0') {
                    // Malformed or extra data? For robustness, stop at comma or end.
                    // If we hit comma, we are good. If we hit something else, we stop.
                    // But standard logic: we just finished a quoted field.
                }
            } else {
                // Unquoted field
                while (*p && *p != ',') {
                    p++;
                }
                if (*p == ',') {
                    p++;
                }
            }
            
            // Calculate length of unescaped content
            const char *scan = field_start;
            size_t content_len = 0;
            if (*scan == '"') {
                scan++;
                while (*scan) {
                    if (*scan == '"') {
                        if (*(scan + 1) == '"') {
                            scan += 2;
                            content_len++;
                        } else {
                            scan++;
                            break;
                        }
                    } else {
                        scan++;
                        content_len++;
                    }
                }
            } else {
                while (*scan && *scan != ',') {
                    scan++;
                    content_len++;
                }
            }
            
            // Copy to output
            size_t copy_len = (content_len < cap - 1) ? content_len : cap - 1;
            if (copy_len > 0) {
                // We need to copy the actual characters. 
                // If we used the 'content_len' logic above, we need to traverse again or store.
                // Let's rewrite the copy loop to be safe and accurate.
            }
            
            // Re-traverse to copy safely
            const char *src = field_start;
            size_t i = 0;
            if (*src == '"') {
                src++;
                while (*src && i < copy_len) {
                    if (*src == '"') {
                        if (*(src + 1) == '"') {
                            out[i++] = '"';
                            src += 2;
                        } else {
                            src++;
                            break;
                        }
                    } else {
                        out[i++] = *src++;
                    }
                }
            } else {
                while (*src && *src != ',' && i < copy_len) {
                    out[i++] = *src++;
                }
            }
            out[i] = '\0';
            
            return (int)content_len;
        }
        
        // Move to next field
        if (*p == '\0') return -1; // idx not found
        
        if (*p == '"') {
            p++;
            while (*p) {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        p += 2;
                    } else {
                        p++;
                        break;
                    }
                } else {
                    p++;
                }
            }
            if (*p == ',') p++;
        } else {
            while (*p && *p != ',') p++;
            if (*p == ',')