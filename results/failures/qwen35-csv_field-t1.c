```c
#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || !out) return -1;
    
    const char *p = line;
    int current_idx = 0;
    int in_quote = 0;
    size_t out_len = 0;
    size_t chars_to_skip = 0;

    // Skip fields until we reach idx
    while (*p) {
        if (in_quote) {
            if (*p == '"') {
                if (*(p + 1) == '"') {
                    p += 2; // Skip doubled quote
                    continue;
                } else {
                    in_quote = 0;
                    p++;
                    continue;
                }
            }
            p++;
        } else {
            if (*p == '"') {
                in_quote = 1;
                p++;
                continue;
            } else if (*p == ',') {
                if (current_idx == idx) {
                    // Found the field, break loop to process it
                    break;
                }
                current_idx++;
                p++;
                continue;
            }
            p++;
        }
    }

    // If we haven't reached idx and reached end of string, check if it's the last field
    if (current_idx < idx && *p == '\0') {
        // If idx is exactly the count of commas + 1 (i.e., the current field we are at)
        // But wait, the loop logic above breaks on comma. Let's re-evaluate the state machine.
        
        // Re-reset logic for robustness
        p = line;
        current_idx = 0;
        in_quote = 0;
        
        while (*p) {
            if (in_quote) {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        p += 2;
                        continue;
                    } else {
                        in_quote = 0;
                        p++;
                        continue;
                    }
                }
                p++;
            } else {
                if (*p == '"') {
                    in_quote = 1;
                    p++;
                    continue;
                } else if (*p == ',') {
                    if (current_idx == idx) {
                        // Found the field, break
                        break;
                    }
                    current_idx++;
                    p++;
                    continue;
                }
                p++;
            }
        }

        // Check if we found the field
        if (current_idx != idx) {
            // If we are at the end of the string and current_idx == idx, it's the last field
            // But the loop breaks on comma or EOL.
            // If we broke on comma, current_idx matches idx.
            // If we hit EOL, we need to check if the field we are currently parsing is idx.
            
            // If the loop finished because *p == '\0', we need to see if current_idx == idx.
            // However, if current_idx != idx after the loop, it means we skipped too many.
            if (current_idx != idx) {
                return -1;
            }
            // If we are here, the field is the remainder of the string.
            // But wait, if we hit EOL, the loop condition *p != 0 fails, loop exits.
            // We need to handle the case where the field is the very last one.
            // The logic "if (current_idx == idx) break" handles the comma case.
            // If no comma exists after the current field, the loop ends.
            // We must check if current_idx == idx after the loop.
        }
    }

    // If we exited because of comma or EOL, and current_idx != idx, then idx is out of range.
    // However, the logic inside the loop increments current_idx only when a comma is processed.
    // If idx is valid, we need to be positioned at the start of that field.
    
    // Let's rewrite the state machine to be clearer:
    p = line;
    current_idx = 0;
    in_quote = 0;
    
    // Skip to the target field
    while (*p && current_idx < idx) {
        if (in_quote) {
            if (*p == '"') {
                if (*(p + 1) == '"') {
                    p += 2;
                    continue;
                } else {
                    in_quote = 0;
                    p++;