#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || idx < 0) return -1;

    const char *p = line;
    int field = 0;
    size_t out_len = 0;
    int in_quotes = 0;

    while (*p) {
        if (in_quotes) {
            if (*p == '"') {
                if (p[1] == '"') {
                    p++; // skip escaped quote
                    if (out_len < cap - 1) out[out_len++] = '"';
                } else {
                    in_quotes = 0; // end of quoted field
                }
            } else {
                if (out_len < cap - 1) out[out_len++] = *p;
            }
        } else {
            if (*p == ',') {
                field++;
                if (field > idx) break;
                out_len = 0; // reset for next field
            } else if (*p == '"') {
                in_quotes = 1;
            } else {
                if (out_len < cap - 1) out[out_len++] = *p;
            }
        }
        p++;
    }

    // Handle last field if not terminated by comma
    if (!in_quotes && field == idx && *p == '\0') {
        // already handled
    } else if (in_quotes && field == idx) {
        // quoted field ended at EOL
        // nothing to do, already processed
    } else if (!in_quotes && field < idx) {
        return -1; // idx out of range
    }

    // If we're still in quotes at EOL, it's malformed but we return what we have
    if (in_quotes && field != idx) {
        return -1;
    }

    // If we exited loop and field == idx, we have the field
    if (field == idx) {
        out[out_len] = '\0';
        return (int)out_len;
    }

    // If we broke due to comma and field == idx+1? No — we break when field > idx
    // Actually, we break when we see a comma and field becomes idx+1 -> meaning we passed the field
    // So if we broke and field == idx+1, then we didn't capture field idx -> out of range
    if (field > idx) {
        return -1;
    }

    // If we reached end and field == idx, we already returned above
    // If field < idx at end -> out of range
    return -1;
}