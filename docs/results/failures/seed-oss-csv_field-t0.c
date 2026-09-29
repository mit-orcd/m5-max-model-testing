#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || !out || cap < 1 || idx < 0) return -1;
    size_t i = 0, field_idx = 0, out_pos = 0;
    char c;

    while ((c = line[i++]) != '\0') {
        if (field_idx > idx) break;
        if (field_idx == idx) {
            if (c == '"') {
                for (; (c = line[i++]) != '\0';) {
                    if (c == '"') {
                        if (line[i] == '"') { i++; continue; }
                        break;
                    }
                    if (out_pos < cap - 1) out[out_pos++] = c;
                }
                if (c != '\0') field_idx++;
            } else {
                if (c == ',') { field_idx++; continue; }
                if (out_pos < cap - 1) out[out_pos++] = c;
            }
        } else if (c == ',') field_idx++;
    }

    if (field_idx <= idx) return -1;
    out[out_pos] = '\0';
    return (int)out_pos;
}