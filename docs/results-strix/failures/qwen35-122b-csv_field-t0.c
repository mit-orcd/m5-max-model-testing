```c
#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (cap == 0) return -1;
    
    int current_idx = 0;
    const char *p = line;
    size_t out_len = 0;
    int in_quotes = 0;
    
    while (*p != '\0') {
        if (current_idx == idx) {
            // We are inside the target field
            if (*p == '"') {
                if (!in_quotes) {
                    // Start of quoted field
                    in_quotes = 1;
                    p++;
                    continue;
                } else {
                    // Inside quoted field
                    if (*(p + 1) == '"') {
                        // Escaped quote
                        if (out_len < cap - 1) {
                            out[out_len++] = '"';
                        }
                        p += 2;
                        continue;
                    } else {
                        // End of quoted field
                        in_quotes = 0;
                        p++;
                        continue;
                    }
                }
            } else if (*p == ',' && !in_quotes) {
                // End of field
                break;
            } else {
                // Regular character
                if (out_len < cap - 1) {
                    out[out_len++] = *p;
                }
                p++;
            }
        } else {
            // Skipping previous fields
            if (*p == '"') {
                if (!in_quotes) {
                    in_quotes = 1;
                } else {
                    if (*(p + 1) == '"') {
                        p += 2;
                        continue;
                    } else {
                        in_quotes = 0;
                    }
                }
            } else if (*p == ',' && !in_quotes) {
                current_idx++;
            }
            p++;
        }
    }
    
    // Check if we found the field
    if (current_idx != idx) {
        // If we reached end of string and current_idx matches, it's the last field
        if (current_idx == idx && out_len > 0) {
            // Already processed
        } else if (current_idx == idx && out_len == 0 && *p == '\0') {
            // Empty last field
        } else {
            return -1;
        }
    }
    
    // Re-scan to get exact length if we were in the target field
    // Actually, the logic above processes the target field. 
    // But we need to handle the case where the field is empty or we just finished it.
    // Let's refine the logic to be more robust.
    
    // Reset and do a cleaner pass
    p = line;
    current_idx = 0;
    out_len = 0;
    in_quotes = 0;
    int found = 0;
    
    while (*p != '\0') {
        if (current_idx == idx) {
            found = 1;
            if (*p == '"') {
                if (!in_quotes) {
                    in_quotes = 1;
                    p++;
                    continue;
                } else {
                    if (*(p + 1) == '"') {
                        if (out_len < cap - 1) {
                            out[out_len++] = '"';
                        }
                        p += 2;
                        continue;
                    } else {
                        in_quotes = 0;
                        p++;
                        continue;
                    }
                }
            } else if (*p == ',' && !in_quotes) {
                break;
            } else {
                if (out_len < cap - 1) {
                    out[out_len++] = *p;
                }
                p++;
            }
        } else {
            if (*p == '"') {
                if (!in_quotes) {
                    in_quotes = 1;
                } else {
                    if (*(p + 1) == '"') {
                        p += 2;
                        continue;
                    } else {
                        in_quotes = 0;
                    }
                }
            } else if (*p == ',' && !in_quotes) {
                current_idx++;
            }
            p++;
        }
    }
    
    if (!found) {
        // Check if it's the last field (empty or not)
        if (current_idx == idx) {
            found = 1;
        }
    }
    
    if (!found) {
        return -1