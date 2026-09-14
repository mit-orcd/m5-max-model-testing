```c
#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || !out || cap == 0) return -1;

    const char *p = line;
    const char *start = NULL;
    const char *end = NULL;
    int current_idx = 0;
    int len = 0;
    int in_quotes = 0;
    int escaped = 0;

    if (idx < 0) return -1;

    // Skip leading whitespace if any (though RFC 4180 doesn't strictly require it, 
    // but usually fields are parsed strictly. We will assume strict field parsing 
    // starting at the beginning or after a delimiter).
    // Actually, RFC 4180 fields start immediately.
    
    while (current_idx < idx) {
        if (*p == '\0') return -1; // End of string before reaching idx

        if (in_quotes) {
            if (escaped) {
                escaped = 0;
            } else if (*p == '"') {
                // Check for doubled quote ""
                if (*(p + 1) == '"') {
                    p += 2;
                    continue;
                } else {
                    // End of quoted field
                    in_quotes = 0;
                    p++;
                    continue;
                }
            }
        } else {
            if (*p == '"') {
                in_quotes = 1;
                p++;
                continue;
            } else if (*p == ',') {
                // Field separator
                current_idx++;
                p++;
                continue;
            }
        }
        p++;
    }

    // Now p points to the start of the desired field
    start = p;
    end = p;

    while (*p != '\0') {
        if (in_quotes) {
            if (escaped) {
                escaped = 0;
            } else if (*p == '"') {
                if (*(p + 1) == '"') {
                    p += 2;
                    continue;
                } else {
                    in_quotes = 0;
                    p++;
                    continue;
                }
            }
        } else {
            if (*p == ',') {
                break;
            }
        }
        p++;
    }

    // Calculate raw length and copy unescaped
    size_t raw_len = (size_t)(p - start);
    size_t out_idx = 0;
    int field_len = 0;

    const char *scan = start;
    while (scan < p) {
        if (in_quotes) {
            if (*scan == '"') {
                if (*(scan + 1) == '"') {
                    if (out_idx < cap - 1) {
                        out[out_idx++] = '"';
                    }
                    field_len++;
                    scan += 2;
                    continue;
                } else {
                    // End of quoted section, but we might still be inside the loop logic
                    // Actually, the loop above handles the boundary. 
                    // If we are here, we are inside quotes and hit the closing quote.
                    // But wait, the loop condition `while (scan < p)` ensures we stop at the comma or end.
                    // The `in_quotes` flag logic inside the copy loop needs to track the state correctly.
                    // Let's re-scan for clarity and correctness.
                }
            }
        }
        scan++;
    }

    // Re-do the copy logic cleanly
    size_t copy_len = 0;
    const char *c = start;
    int quote_state = 0; // 0: normal, 1: inside quotes, 2: seen quote (checking for "")
    
    // Reset state for the specific field
    quote_state = 0;
    // We need to know if the field started with a quote to set initial quote_state
    // Actually, the 'in_quotes' logic from the skip loop was correct for determining boundaries.
    // But we need to track it again for the copy.
    // Let's just use the pointer logic from the skip loop to determine if the field was quoted.
    // A simpler way: re-traverse from start.
    
    const char *scan_ptr = start;
    int field_started_quoted = 0;
    if (*scan_ptr == '"') field_started_quoted = 1;
    
    int in_q = field_started_quoted;
    int esc = 0;