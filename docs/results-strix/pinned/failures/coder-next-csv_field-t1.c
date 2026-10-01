#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) {
        return -1;
    }

    size_t pos = 0;  // current position in line
    int field_num = 0;  // current field index
    size_t out_len = 0;  // length of output string

    // Skip leading whitespace (not required by RFC-4180, but common practice)
    while (line[pos] == ' ' || line[pos] == '\t') {
        pos++;
    }

    while (field_num <= idx) {
        // Check for end of line
        if (line[pos] == '\0') {
            return -1;
        }

        // Handle empty field
        if (line[pos] == ',') {
            if (field_num == idx) {
                out[0] = '\0';
                return 0;
            }
            pos++;
            field_num++;
            continue;
        }

        // Handle quoted field
        if (line[pos] == '"') {
            pos++;  // skip opening quote
            size_t field_start = pos;
            int in_quotes = 1;

            while (in_quotes && line[pos] != '\0') {
                if (line[pos] == '"') {
                    if (line[pos + 1] == '"') {
                        // Escaped quote
                        if (field_num == idx && out_len + 1 < cap) {
                            out[out_len++] = '"';
                        }
                        pos += 2;
                    } else {
                        // End of quoted field
                        in_quotes = 0;
                        pos++;
                    }
                } else {
                    if (field_num == idx && out_len + 1 < cap) {
                        out[out_len++] = line[pos];
                    }
                    pos++;
                }
            }

            // Skip trailing whitespace after closing quote (not required by RFC-4180)
            while (line[pos] == ' ' || line[pos] == '\t') {
                pos++;
            }

            // Check for comma or end of line
            if (line[pos] == ',') {
                pos++;
            } else if (line[pos] != '\0' && line[pos] != '\r' && line[pos] != '\n') {
                // Invalid character after quoted field
                return -1;
            }

            if (field_num == idx) {
                out[out_len] = '\0';
                return (int)out_len;
            }
            field_num++;
        } else {
            // Unquoted field
            size_t field_start = pos;
            while (line[pos] != ',' && line[pos] != '\0' && 
                   line[pos] != '\r' && line[pos] != '\n') {
                if (field_num == idx && out_len + 1 < cap) {
                    out[out_len++] = line[pos];
                }
                pos++;
            }

            if (field_num == idx) {
                out[out_len] = '\0';
                return (int)out_len;
            }
            field_num++;

            // Skip comma
            if (line[pos] == ',') {
                pos++;
            }
        }
    }

    return -1;
}