```c
#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || !out || cap == 0) return -1;
    
    const char *p = line;
    int current_idx = 0;
    size_t out_len = 0;
    int in_quotes = 0;
    int field_start = 1; // Flag to track if we are at the start of a field

    while (*p) {
        if (current_idx == idx) {
            // We are processing the target field
            if (!in_quotes) {
                if (*p == ',') {
                    // End of field
                    if (out_len > 0 && out[out_len - 1] == '"') {
                        out_len--; // Remove trailing quote if present (unquoted comma case handled differently)
                    }
                    break;
                }
            } else {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        // Escaped quote
                        p++;
                        if (out_len < cap - 1) {
                            out[out_len++] = '"';
                        }
                    } else {
                        // End of quoted field
                        in_quotes = 0;
                        if (*(p + 1) == ',') {
                            // End of field
                            break;
                        }
                    }
                } else {
                    if (out_len < cap - 1) {
                        out[out_len++] = *p;
                    }
                }
            }
        } else {
            // Skip other fields
            if (!in_quotes) {
                if (*p == '"') {
                    in_quotes = 1;
                } else if (*p == ',') {
                    current_idx++;
                    if (current_idx > idx) break;
                }
            } else {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        p++;
                    } else {
                        in_quotes = 0;
                    }
                }
            }
        }
        
        if (current_idx == idx && in_quotes && *p == '"') {
            // Handle start of quoted field immediately
            if (*(p + 1) == '"') {
                // This is an escaped quote at the start of a quoted field? 
                // Actually RFC 4180: field starts with quote, then content.
                // If we are inside the loop checking current_idx, we need to be careful.
                // The logic above handles skipping. Let's refine the skip logic.
            }
        }
        p++;
    }

    // Re-implementing with a cleaner single pass state machine
    // Reset state
    p = line;
    current_idx = 0;
    out_len = 0;
    in_quotes = 0;
    int field_skipping = 1; // True if we are currently skipping fields before target

    while (*p && (field_skipping || current_idx <= idx)) {
        if (field_skipping) {
            if (!in_quotes) {
                if (*p == '"') {
                    in_quotes = 1;
                } else if (*p == ',') {
                    current_idx++;
                    if (current_idx == idx) {
                        field_skipping = 0;
                    }
                }
            } else {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        p++; // Skip next quote
                    } else {
                        in_quotes = 0;
                        if (*(p + 1) == ',') {
                            current_idx++;
                            if (current_idx == idx) {
                                field_skipping = 0;
                            }
                        }
                    }
                }
            }
        } else {
            // Extracting
            if (!in_quotes) {
                if (*p == ',') {
                    break;
                }
                if (out_len < cap - 1) {
                    out[out_len++] = *p;
                }
            } else {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        p++;
                        if (out_len < cap - 1) {
                            out[out_len++] = '"';
                        }
                    } else {
                        in_quotes = 0;
                        if (*(p + 1) == ',') {
                            // We are at the end of the field
                            break;
                        }
                    }