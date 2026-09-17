#include <stddef.h>
#include <string.h>
#include <stdbool.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || !out || cap == 0) return -1;
    if (idx < 0) return -1;

    const char *p = line;
    int current_field = 0;
    size_t out_pos = 0;
    bool in_quotes = false;
    bool field_started = false;

    while (true) {
        if (!field_started) {
            if (*p == '"') {
                in_quotes = true;
                field_started = true;
                p++;
                continue;
            } else if (*p == ',' || *p == '\0') {
                if (current_field == idx) {
                    if (out_pos < cap) out[out_pos] = '\0';
                    return (int)out_pos;
                }
                current_field++;
                out_pos = 0;
                field_started = false;
                if (*p == '\0') break;
                p++;
                continue;
            } else {
                field_started = true;
            }
        }

        if (in_quotes) {
            if (*p == '"') {
                if (*(p + 1) == '"') {
                    if (out_pos < cap - 1) out[out_pos++] = '"';
                    p += 2;
                } else {
                    in_quotes = false;
                    p++;
                }
            } else if (*p == '\0') {
                break;
            } else {
                if (out_pos < cap - 1) out[out_pos++] = *p;
                p++;
            }
        } else {
            if (*p == ',') {
                if (current_field == idx) {
                    if (out_pos < cap) out[out_pos] = '\0';
                    return (int)out_pos;
                }
                current_field++;
                out_pos = 0;
                field_started = false;
                p++;
                continue;
            } else if (*p == '\0') {
                if (current_field == idx) {
                    if (out_pos < cap) out[out_pos] = '\0';
                    return (int)out_pos;
                }
                break;
            } else {
                if (out_pos < cap - 1) out[out_pos++] = *p;
                p++;
            }
        }
    }

    return -1;
}