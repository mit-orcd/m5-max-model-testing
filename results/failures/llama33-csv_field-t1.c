#include <stdbool.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    bool in_quote = false;
    bool field_started = false;
    int field_idx = 0;
    size_t out_idx = 0;

    for (const char *c = line; *c; ++c) {
        if (!field_started) {
            if (*c == '\"') {
                in_quote = true;
                field_started = true;
            } else if (*c == ',') {
                ++field_idx;
            } else if (*c != ' ' && *c != '\t') {
                field_started = true;
            }
        } else {
            if (in_quote) {
                if (*c == '\"') {
                    in_quote = false;
                } else {
                    if (out_idx < cap - 1 && field_idx == idx) {
                        out[out_idx++] = *c;
                    }
                }
            } else {
                if (*c == ',') {
                    field_started = false;
                    in_quote = false;
                    ++field_idx;
                    if (field_idx == idx) {
                        out[out_idx] = '\0';
                        return out_idx;
                    }
                } else if (*c == '\"') {
                    in_quote = true;
                } else if (out_idx < cap - 1 && field_idx == idx) {
                    out[out_idx++] = *c;
                }
            }
        }
    }

    if (field_idx == idx) {
        out[out_idx] = '\0';
        return out_idx;
    } else {
        return -1;
    }
}