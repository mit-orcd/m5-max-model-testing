```c
#include <stddef.h>
#include <stdbool.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) {
        return -1;
    }

    const char *p = line;
    int current_idx = 0;
    size_t out_len = 0;
    bool in_quotes = false;
    bool field_started = false;

    while (*p != '\0') {
        char c = *p;

        if (!in_quotes) {
            if (c == '"') {
                in_quotes = true;
                field_started = true;
                p++;
                continue;
            } else if (c == ',') {
                if (field_started) {
                    current_idx++;
                    field_started = false;
                }
                p++;
                continue;
            } else {
                field_started = true;
            }
        } else {
            if (c == '"') {
                if (*(p + 1) == '"') {
                    // Escaped quote
                    if (out_len < cap - 1) {
                        out[out_len++] = '"';
                    }
                    p += 2;
                    continue;
                } else {
                    // End of quoted field
                    in_quotes = false;
                    p++;
                    continue;
                }
            } else {
                if (out_len < cap - 1) {
                    out[out_len++] = c;
                }
                p++;
                continue;
            }
        }

        // Unquoted character processing
        if (current_idx == idx) {
            if (out_len < cap - 1) {
                out[out_len++] = c;
            }
        }
        p++;
    }

    // Handle the last field if we are still inside it
    if (current_idx == idx) {
        out[out_len] = '\0';
        return (int)out_len;
    }

    // Check if we reached the end and the requested index was not found
    // We need to verify if the last field was the one we wanted or if we ran out
    // The loop above increments current_idx on commas. 
    // If we finished the loop and current_idx <= idx, we need to check the last field logic again.
    // Actually, the logic above processes characters. Let's refine the state tracking.
    
    // Re-evaluating logic: The previous loop structure was slightly flawed for the last field.
    // Let's restart with a cleaner state machine approach.
    
    p = line;
    current_idx = 0;
    out_len = 0;
    in_quotes = false;
    bool collecting = false;
    
    while (*p != '\0') {
        char c = *p;
        
        if (!in_quotes) {
            if (c == '"') {
                in_quotes = true;
                collecting = true;
                p++;
                continue;
            } else if (c == ',') {
                if (collecting) {
                    current_idx++;
                    collecting = false;
                }
                p++;
                continue;
            } else {
                collecting = true;
            }
        } else {
            if (c == '"') {
                if (*(p + 1) == '"') {
                    if (current_idx == idx) {
                        if (out_len < cap - 1) {
                            out[out_len++] = '"';
                        }
                    }
                    p += 2;
                    continue;
                } else {
                    in_quotes = false;
                    p++;
                    continue;
                }
            } else {
                if (current_idx == idx) {
                    if (out_len < cap - 1) {
                        out[out_len++] = c;
                    }
                }
                p++;
                continue;
            }
        }
        
        // Unquoted char
        if (current_idx == idx) {
            if (out_len < cap - 1) {
                out[out_len++] = c;
            }
        }
        p++;
    }
    
    // Check if the last field was the target
    // If we are still in_quotes, the line is malformed per RFC 4180, but we treat it as end of field.
    // If we finished a field, current_idx is the index of the field we just finished.
    // If we are currently collecting (unquoted or quoted), current_idx