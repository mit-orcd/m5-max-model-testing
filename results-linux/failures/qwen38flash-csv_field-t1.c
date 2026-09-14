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
    int found = 0;

    while (1) {
        // Parse the current field
        const char *field_start = p;
        int is_quoted = 0;

        if (*p == '"') {
            is_quoted = 1;
            p++; // skip opening quote
            // Scan until closing quote
            while (*p) {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        // Escaped quote
                        p += 2;
                    } else {
                        // Closing quote
                        p++;
                        break;
                    }
                } else {
                    p++;
                }
            }
            // After closing quote, expect comma or end of line
            if (*p == ',') {
                p++; // skip comma, move to next field
                // The actual content for unescaping starts after the first quote
            } else if (*p == '\0') {
                // End of line
            } else {
                // Malformed, but we'll treat it as end of field
                // Actually, in strict RFC, after closing quote must be comma or EOL
                // If not, this is an error, but we'll just stop parsing this field
            }
        } else {
            // Unquoted field
            while (*p && *p != ',') {
                p++;
            }
            if (*p == ',') {
                // Will skip comma after processing
            }
        }

        // Check if this is the target field
        if (current_idx == idx) {
            found = 1;
            // Extract the field content
            const char *content_start;
            const char *content_end;
            size_t raw_len;

            if (is_quoted) {
                // content_start is after opening quote
                // We need to re-scan to find the actual content boundaries
                // Actually, we already advanced p past the closing quote
                // Let's re-derive content_start and content_end
                // field_start points to the opening quote
                content_start = field_start + 1;
                // content_end is before the closing quote
                // p is now after the closing quote (or after comma)
                // We need to find the position of the closing quote
                // Let's re-scan from field_start+1
                const char *q = field_start + 1;
                while (*q) {
                    if (*q == '"') {
                        if (*(q + 1) == '"') {
                            q += 2;
                        } else {
                            break;
                        }
                    } else {
                        q++;
                    }
                }
                content_end = q;
                raw_len = (size_t)(content_end - content_start);
            } else {
                content_start = field_start;
                // p points to comma or end of line
                // If *p == ',', content_end is p (the comma is not part of field)
                // If *p == '\0', content_end is p
                content_end = p;
                if (*p == ',') {
                    content_end = p;
                }
                raw_len = (size_t)(content_end - content_start);
            }

            // Now unescape into out
            size_t i = 0;
            for (size_t j = 0; j < raw_len && out_len < cap - 1; j++) {
                char c = content_start[j];
                if (is_quoted && c == '"' && j + 1 < raw_len && content_start[j + 1] == '"') {
                    // Doubled quote -> single quote
                    out[out_len++] = '"';
                    j++; // skip the next quote
                } else {
                    out[out_len++] = c;
                }
            }
            out[out_len] = '\0';
            // The unescaped length is out_len
            return (int)out_len;
        }

        // Move to next field
        if (!found) {
            if (*p == ',') {
                // Already advanced past comma in some cases