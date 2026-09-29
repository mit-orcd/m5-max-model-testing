#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) {
        return -1;
    }

    size_t pos = 0;      // current position in line
    int field_num = 0;   // current field index
    size_t out_len = 0;  // length of output string (unescaped)
    int in_quotes = 0;   // whether we're inside a quoted field

    while (line[pos] != '\0' && field_num <= idx) {
        // Start of a new field
        if (field_num == idx) {
            // Process current field
            size_t out_pos = 0;

            if (line[pos] == '"') {
                in_quotes = 1;
                pos++;
            }

            while (line[pos] != '\0') {
                if (in_quotes) {
                    if (line[pos] == '"') {
                        // Check for doubled quote
                        if (line[pos + 1] == '"') {
                            if (out_pos < cap - 1) {
                                out[out_pos++] = '"';
                            }
                            out_len++;
                            pos += 2;
                            continue;
                        } else {
                            // End of quoted field
                            in_quotes = 0;
                            pos++;
                            break;
                        }
                    } else {
                        if (out_pos < cap - 1) {
                            out[out_pos++] = line[pos];
                        }
                        out_len++;
                        pos++;
                    }
                } else {
                    // Unquoted field: ends at comma or newline
                    if (line[pos] == ',' || line[pos] == '\r' || line[pos] == '\n') {
                        break;
                    }
                    if (out_pos < cap - 1) {
                        out[out_pos++] = line[pos];
                    }
                    out_len++;
                    pos++;
                }
            }

            // Null-terminate
            out[out_pos] = '\0';

            // If we've reached the end of the field, return its length
            return (int)out_len;
        }

        // Skip to next field
        if (line[pos] == '"') {
            in_quotes = 1;
            pos++;
            while (line[pos] != '\0' && (line[pos] != '"' || (line[pos] == '"' && line[pos + 1] == '"'))) {
                if (line[pos] == '"') {
                    pos++; // skip the first quote of doubled quote
                }
                pos++;
            }
            if (line[pos] == '"') {
                pos++; // skip closing quote
            }
        } else {
            while (line[pos] != '\0' && line[pos] != ',' && line[pos] != '\r' && line[pos] != '\n') {
                pos++;
            }
        }

        // Skip comma (if present)
        if (line[pos] == ',') {
            pos++;
        }

        field_num++;
    }

    // idx was out of range
    return -1;
}