#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    const char *start = strchr(line, ',');
    if (start && idx == 0)
        start = line;
    else {
        int i = 0;
        while (i < idx && start = strchr(start + 1, ','))
            i++;
        if (start)
            start++;
        else
            return -1;
    }
    const char *end = start;
    while (*end != ',' && *end != '\r' && *end != '\n' && end < line + strlen(line))
        end++;
    if (*end == '\r')
        end++;
    size_t len = end - start;
    if (len + 1 > cap)
        len = cap - 1;
    memcpy(out, start, len);
    out[len] = '\0';
    return len;
}