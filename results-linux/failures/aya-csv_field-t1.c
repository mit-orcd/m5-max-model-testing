#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (idx < 0 || idx >= (int)strcspn(line, ","))
        return -1;
    const char *start = strchr(line, ',') + 1;
    size_t len = strcspn(start, ",\"") - 1;
    if (len + 1 > cap)
        len = cap - 1;
    memcpy(out, start, len);
    out[len] = '\0';
    return (int)len;
}