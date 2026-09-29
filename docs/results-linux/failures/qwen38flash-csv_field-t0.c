```c
#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || !out || cap == 0) {
        if (out && cap > 0) out[0] = '\0';
        return -1;
    }

    const char *p = line;
    int current_idx = 0;
    size_t out_len = 0;

    while (1) {
        // Parse one field
        const char *field_start = p;
        int quoted = 0;
        size_t field_len = 0; // unescaped length of this field

        if (*p == '"') {
            quoted = 1;
            p++; // skip opening quote
            // Read until closing quote
            while (*p) {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        // doubled quote -> one quote
                        field_len++;
                        p += 2;
                    } else {
                        // end of quoted field
                        p++; // skip closing quote
                        break;
                    }
                } else {
                    field_len++;
                    p++;
                }
            }
            // After closing quote, expect comma or end of line
            if (*p == ',') {
                p++;
            } else if (*p != '\0') {
                // Malformed, but we'll treat as end
                // Actually, just break
            }
        } else {
            // Unquoted field: read until comma or end
            while (*p && *p != ',') {
                field_len++;
                p++;
            }
            if (*p == ',') {
                p++;
            }
        }

        // Check if this is the target field
        if (current_idx == idx) {
            // Copy field content to out
            const char *src = field_start;
            size_t remaining = field_len;
            size_t copy_len = (remaining < cap - 1) ? remaining : cap - 1;

            if (quoted) {
                // Need to unescape: skip opening quote, handle doubled quotes
                src = field_start + 1; // skip opening "
                size_t i = 0;
                while (i < copy_len && *src) {
                    if (*src == '"') {
                        if (*(src + 1) == '"') {
                            out[out_len++] = '"';
                            src += 2;
                        } else {
                            // end of quoted field
                            break;
                        }
                    } else {
                        out[out_len++] = *src;
                        src++;
                    }
                }
                // If we stopped early due to cap, out_len == copy_len
                // But we need to make sure we don't exceed cap-1
                if (out_len >= cap) {
                    out_len = cap - 1;
                }
            } else {
                // Unquoted: just copy
                size_t i = 0;
                while (i < copy_len && *src) {
                    out[out_len++] = *src;
                    src++;
                }
            }
            out[out_len] = '\0';
            return (int)field_len;
        }

        current_idx++;

        // If we reached end of line and haven't found the field, it's out of range
        if (*p == '\0') {
            // Check if there was a trailing comma creating an empty field
            // Actually, if we just processed a field and p is at end, no more fields
            // But if the line ended with a comma, there's an empty field after it
            // Let's check: after parsing a field, if we consumed a comma, p points to next field start
            // If p is at end after consuming comma, that means there's an empty field
            // Hmm, let me reconsider.
            // If line is "a,b,", after parsing "b", p points to empty string after comma.
            // So we need to check if we just consumed a comma and p is at end.
            // Actually, the loop structure: after parsing field, if *p == '\0', we break.
            // But if the line ends with comma, after parsing the last non-empty field,
            // p points to '\0' after the comma was consumed? No.
            // Let me trace: "a,b,"
            // Start: p = "a,b,"
            // Parse field 0: unquoted, reads 'a', stops