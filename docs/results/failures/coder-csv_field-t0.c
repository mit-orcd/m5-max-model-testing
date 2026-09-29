#include <string.h>
#include <stdio.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || !out || cap == 0) return -1;
    
    int field_idx = 0;
    const char *start = line;
    const char *end = line;
    int in_quotes = 0;
    int escaped = 0;
    
    while (*end) {
        if (!in_quotes && *end == ',') {
            if (field_idx == idx) {
                size_t len = end - start;
                if (len >= cap) len = cap - 1;
                strncpy(out, start, len);
                out[len] = '\0';
                return len;
            }
            field_idx++;
            start = end + 1;
        } else if (*end == '"') {
            if (!in_quotes && *(end + 1) == '"') {
                // Double quote inside field - skip one
                end++;
            } else if (!in_quotes) {
                in_quotes = 1;
            } else if (in_quotes && *(end + 1) == '"') {
                // Double quote inside field - skip one
                end++;
            } else {
                in_quotes = 0;
            }
        } else if (*end == '\r' || *end == '\n') {
            // End of line
            if (field_idx == idx) {
                size_t len = end - start;
                if (len >= cap) len = cap -1;
                strncpy(out, start, len);
                out[len] = '\0';
                return len;
            }
            break;
        }
        end++;
    }
    
    // Handle last field
    if (field_idx == idx) {
        size_t len = end - start;
        if (len >= cap) len = cap - 1;
        strncpy(out, start, len);
        out[len] = '\0';
        return len;
    }
    
    return -1;
}