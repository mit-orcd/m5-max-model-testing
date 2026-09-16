#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || !out || cap < 1 || idx < 0) return -1;
    size_t i = 0, field_idx = 0, out_pos = 0;
    int inquotes = 0, escaped = 0;

    while (line[i]) {
        char c = line[i++];
        if (escaped) {
            out[out_pos++] = c;
            escaped = 0;
            if (out_pos == cap - 1) break;
        } else if (c == '"') {
            if (line[i] == '"') {
                escaped = 1;
                i++;
            } else {
                inquotes = !inquotes;
            }
        } else if (c == ',' && !inquotes) {
            if (field_idx == idx) break;
            field_idx++;
            out_pos = 0;
        } else {
            if (field_idx == idx) {
                out[out_pos++] = c;
                if (out_pos == cap - 1) break;
            }
        }
    }

    if (field_idx < idx) return -1;
    out[out_pos] = '\0';
    return (int)out_pos;
}