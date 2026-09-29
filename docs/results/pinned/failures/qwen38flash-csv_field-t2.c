```c
#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || !out || cap == 0) {
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
            // Scan until closing quote
            while (*p) {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        // doubled quote, counts as one char in unescaped
                        field_len++;
                        p += 2;
                    } else {
                        // closing quote
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
                // Malformed, but we'll just continue
                // Actually, per RFC, after closing quote should be comma or EOL
                // If not, treat as part of field? Let's be lenient and just move on.
                // But for correctness, if it's not comma or EOL, it's invalid.
                // We'll just break out of parsing this field.
            }
        } else {
            // Unquoted field
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
            // Extract the field content into out
            const char *src = field_start;
            size_t remaining = field_len;
            size_t copy_len = (remaining < cap - 1) ? remaining : cap - 1;

            if (quoted) {
                // Skip opening quote
                src++;
                size_t copied = 0;
                while (copied < copy_len && *src) {
                    if (*src == '"') {
                        if (*(src + 1) == '"') {
                            out[out_len++] = '"';
                            src += 2;
                            copied++;
                        } else {
                            // closing quote, stop
                            break;
                        }
                    } else {
                        out[out_len++] = *src;
                        src++;
                        copied++;
                    }
                }
                // If we stopped early due to closing quote but need more chars,
                // this shouldn't happen if field_len is correct.
                // But if copy_len < field_len, we truncated.
                // Ensure we don't exceed cap-1
                if (out_len >= cap) {
                    out_len = cap - 1;
                }
            } else {
                // Unquoted: just copy
                size_t to_copy = (field_len < cap - 1) ? field_len : cap - 1;
                memcpy(out, src, to_copy);
                out_len = to_copy;
            }

            out[out_len] = '\0';
            return (int)field_len;
        }

        current_idx++;

        // If we reached end of line and haven't found the field, it's out of range
        if (*p == '\0') {
            // Check if there was a trailing comma creating an empty field
            // Actually, if we just parsed a field and p is at '\0', 
            // and current_idx > idx, then idx is out of range.
            // But what if the line ends with a comma? Then there's an empty field after it.
            // Let's handle that: if the last character before '\0' was a comma,
            // there's one more empty field.
            // Actually, our loop structure: after parsing a field, if *p == '\0', we break.
            // But if the line ends with ',', then after parsing the last non-empty field,
            // p points to '\0' after consuming the comma? No.
            // Let's trace: "a,b," 
            // Parse "a": p moves past 'a', sees ',', p