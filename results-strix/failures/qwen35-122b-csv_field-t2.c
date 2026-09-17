```c
#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    size_t out_len = 0;
    int current_idx = 0;
    const char *p = line;
    int in_quotes = 0;
    int field_started = 0;
    int field_idx_match = 0;

    if (idx < 0 || cap == 0) {
        return -1;
    }

    while (*p != '\0') {
        if (!field_started) {
            current_idx++;
            if (current_idx == idx + 1) {
                field_idx_match = 1;
            } else if (current_idx > idx + 1) {
                return -1;
            }
            field_started = 1;
        }

        if (current_idx == idx + 1) {
            if (*p == '"') {
                if (!in_quotes) {
                    in_quotes = 1;
                } else {
                    // Check for escaped quote
                    if (*(p + 1) == '"') {
                        if (out_len + 1 < cap) {
                            out[out_len++] = '"';
                        }
                        p++; // Skip next quote
                    } else {
                        // End of quoted field
                        in_quotes = 0;
                    }
                }
            } else if (*p == ',' && !in_quotes) {
                // End of field
                break;
            } else {
                if (out_len + 1 < cap) {
                    out[out_len++] = *p;
                }
            }
        } else {
            // Skip characters in previous fields
            if (*p == '"') {
                if (!in_quotes) {
                    in_quotes = 1;
                } else {
                    if (*(p + 1) == '"') {
                        p++;
                    } else {
                        in_quotes = 0;
                    }
                }
            } else if (*p == ',' && !in_quotes) {
                // Move to next field
                field_started = 0;
            }
        }
        p++;
    }

    // Check if we are still in the target field at end of string
    if (current_idx == idx + 1 && !field_started) {
        // This case shouldn't happen due to logic flow, but handle end of string
    }

    // If we never found the field index (e.g., line was empty and idx > 0)
    if (idx > 0 && !field_started && current_idx < idx + 1) {
         // Need to verify if we reached the end without finding the field
         // The loop logic increments current_idx at start of field processing
         // If line is "a,b", idx=2:
         // p='a': started=1, curr=1. p=',': started=0.
         // p='b': started=1, curr=2. p='\0': loop ends.
         // current_idx is 2. idx+1 is 3. So 2 < 3.
         // We need to handle the case where the field doesn't exist.
    }
    
    // Re-evaluate logic for "not found"
    // If we are at the end of the string and current_idx <= idx, then the field doesn't exist
    // unless we are currently processing the target field (which we are if current_idx == idx+1)
    // If current_idx < idx+1, it means we ran out of fields.
    
    if (current_idx < idx + 1) {
        return -1;
    }

    out[out_len] = '\0';
    
    // Calculate the unescaped length of the field
    // We need to recount the actual unescaped length of the target field
    // because we truncated during copy if cap was small.
    // Or we can just trust out_len if we didn't need to recount? 
    // The spec says "Returns the field's unescaped length".
    // If we truncated, out_len is cap-1, but the real length might be larger.
    // So we must calculate the real unescaped length.

    size_t real_len = 0;
    const char *q = line;
    int q_in_quotes = 0;
    int q_field_idx = 0;
    int q_field_found = 0;

    while (*q != '\0') {
        if (!q_field_found) {
            q_field_idx++;
            if (q_field_idx == idx