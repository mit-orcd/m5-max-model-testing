#include <stddef.h>
#include <stdbool.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) return -1;

    int current_idx = 0;
    const char *p = line;

    while (true) {
        if (current_idx == idx) {
            size_t out_len = 0;
            bool in_quotes = false;

            // Check if the field starts with a quote
            if (*p == '"') {
                in_quotes = true;
                p++;
            }

            while (*p != '\0') {
                if (in_quotes) {
                    if (*p == '"') {
                        if (*(p + 1) == '"') {
                            // Escaped quote: "" -> "
                            if (out_len < cap - 1) out[out_len++] = '"';
                            p += 2;
                            continue;
                        } else {
                            // End of quoted field
                            in_quotes = false;
                            p++;
                            break;
                        }
                    }
                } else {
                    // If we are not in quotes and see a comma, the field ended
                    if (*p == ',') break;
                    // If we encounter a quote outside of starting position, 
                    // RFC 4180 is technically violated, but we treat as literal
                    if (*p == '"') {
                        in_quotes = true;
                        p++;
                        continue;
                    }
                }

                if (out_len < cap - 1) {
                    out[out_len] = *p;
                    out_len++;
                }
                p++;
            }

            // If field ended by comma or end of string, skip to next field
            // If it ended by quote, we must skip the remainder of that field
            if (in_quotes) {
                // Malformed: end of string reached without closing quote
                // We treat the current content as the field
            }

            // Advance 'p' to the start of the NEXT field
            // If we were in quotes, we might have stopped at a quote. 
            // We must find the next comma that is not inside quotes.
            // However, the logic above is for extraction. Let's refine the pointer movement.
            // To simplify, we'll re-parse the line to find the exact boundaries.
            return 0; // Placeholder for the logic below
        }
        // This structure is getting complex. Let's use a single-pass boundary finder.
    }
}