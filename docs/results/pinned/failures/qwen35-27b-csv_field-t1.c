```c
#include <stddef.h>
#include <stdbool.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) {
        return -1;
    }

    const char *p = line;
    int current_idx = 0;
    bool in_quotes = false;
    bool field_started = false;
    size_t out_len = 0;

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
                if (current_idx == idx) {
                    // Start of target field
                    break;
                }
                p++;
                continue;
            } else {
                field_started = true;
                if (current_idx == idx) {
                    // Start of target field (unquoted)
                    break;
                }
                p++;
                continue;
            }
        } else {
            // Inside quotes
            if (c == '"') {
                if (*(p + 1) == '"') {
                    // Escaped quote
                    if (current_idx == idx && out_len < cap - 1) {
                        out[out_len++] = '"';
                    }
                    p += 2;
                    continue;
                } else {
                    // End of quoted field
                    in_quotes = false;
                    p++;
                    break;
                }
            } else {
                if (current_idx == idx && out_len < cap - 1) {
                    out[out_len++] = c;
                }
                p++;
            }
        }
    }

    // If we broke out because we found the start of the target field
    if (current_idx == idx) {
        // If we broke out at a comma or quote, we need to process the rest
        // Re-evaluate logic: The break above happens when we identify the start.
        // We need to continue parsing from the current position 'p'
        
        // Reset out_len for the actual extraction loop
        out_len = 0;
        
        // If we broke out at a quote, we are inside quotes now
        // If we broke out at a comma, we are outside quotes
        // The logic above was slightly flawed in the break condition.
        // Let's restart the parsing logic cleanly.
    }

    // Clean implementation restart
    p = line;
    current_idx = 0;
    in_quotes = false;
    out_len = 0;
    bool found = false;

    while (*p != '\0') {
        char c = *p;

        if (!in_quotes) {
            if (c == '"') {
                in_quotes = true;
                p++;
                continue;
            } else if (c == ',') {
                current_idx++;
                p++;
                continue;
            } else {
                // Unquoted field start
                if (current_idx == idx) {
                    found = true;
                    // Process this char
                    if (out_len < cap - 1) {
                        out[out_len++] = c;
                    }
                    p++;
                    continue;
                }
                // Skip chars for non-target fields
                p++;
                continue;
            }
        } else {
            // Inside quotes
            if (c == '"') {
                if (*(p + 1) == '"') {
                    // Escaped quote
                    if (current_idx == idx && out_len < cap - 1) {
                        out[out_len++] = '"';
                    }
                    p += 2;
                    continue;
                } else {
                    // End of quoted field
                    in_quotes = false;
                    p++;
                    // Check if this was the end of the target field
                    if (current_idx == idx) {
                        found = true;
                        break;
                    }
                    continue;
                }
            } else {
                if (current_idx == idx && out_len < cap - 1) {
                    out[out_len++] = c;
                }
                p++;
                continue;
            }
        }
    }

    // Handle case where the last field ends at EOF
    if (current_idx == idx && !found) {