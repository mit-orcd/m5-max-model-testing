#include <string.h>
#include <stdio.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || !out || cap == 0) return -1;
    
    int field_idx = 0;
    const char *start = line;
    const char *pos = line;
    int in_quotes = 0;
    int escaped = 0;
    
    while (*pos) {
        if (!in_quotes && *pos == '"') {
            in_quotes = 1;
        } else if (in_quotes && *pos == '"') {
            if (*(pos + 1) == '"') {
                pos++;
            } else {
                in_quotes = 0;
            }
        } else if (!in_quotes && *pos == ',') {
            if (field_idx == idx) {
                size_t len = pos - start;
                if (len >= cap) len = cap - 1;
                strncpy(out, start, len);
                out[len] = '\0';
                return len;
            }
            start = pos + 1;
            field_idx++;
        }
        pos++;
    }
    
    if (field_idx == idx) {
        size_t len = pos - start;
        if (len >= cap) len = cap - 1;
        strncpy(out, start, len);
        out[len] = '\0';
        return len;
    }
    
    return -1;
}