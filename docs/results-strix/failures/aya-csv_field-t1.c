#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    size_t len = strlen(line);
    if (idx < 0 || idx >= len) return -1;

    const char *start = line + idx;
    while (start < line + len && *start != ',' && *start != '"') start++;
    size_t field_len = start - line - idx;

    if (field_len >= cap) {
        memcpy(out, line + idx, cap - 1);
        out[cap - 1] = '\0';
        return cap - 1;
    }

    memcpy(out, line + idx, field_len);
    out[field_len] = '\0';
    return field_len;
}