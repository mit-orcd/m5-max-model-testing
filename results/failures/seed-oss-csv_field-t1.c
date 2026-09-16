#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || !out || cap < 1 || idx < 0) return -1;
    size_t i = 0, field_start = 0, field_len = 0;
    int in_quotes = 0, current_field = 0;

    while (line[i] != '\0') {
        char c = line[i];
        if (c == '"') {
            in_quotes = !in_quotes;
            i++;
            if (in_quotes) continue;
            if (line[i] == '"') { out[field_len++] = '"'; i++; }
            else { out[field_len] = '\0'; return (int)field_len; }
        }
        if (!in_quotes) {
            if (c == ',') {
                if (current_field == idx) { out[field_len] = '\0'; return (int)field_len; }
                current_field++; field_start = i + 1; field_len = 0;
            } else {
                if (current_field == idx) {
                    if (field_len < cap - 1) out[field_len] = c;
                    field_len++;
                }
            }
        }
        i++;
    }
    if (current_field == idx) { out[field_len] = '\0'; return (int)field_len; }
    return -1;
}