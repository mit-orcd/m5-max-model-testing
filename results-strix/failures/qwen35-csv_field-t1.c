#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || !out || cap == 0) return -1;
    
    size_t len = 0;
    int field_count = 0;
    size_t i = 0;
    size_t line_len = 0;
    
    while (line[i] != '\0') line_len++;

    while (i <= line_len) {
        if (field_count == idx) {
            size_t start = i;
            int in_quotes = 0;
            
            if (line[i] == '"') {
                in_quotes = 1;
                i++;
            }
            
            while (i < line_len) {
                if (in_quotes) {
                    if (line[i] == '"') {
                        if (i + 1 < line_len && line[i+1] == '"') {
                            if (len + 1 < cap) {
                                out[len++] = '"';
                            }
                            i += 2;
                        } else {
                            in_quotes = 0;
                            i++;
                            break;
                        }
                    } else {
                        if (len + 1 < cap) {
                            out[len++] = line[i];
                        }
                        i++;
                    }
                } else {
                    if (line[i] == ',') {
                        break;
                    }
                    if (len + 1 < cap) {
                        out[len++] = line[i];
                    }
                    i++;
                }
            }
            out[len] = '\0';
            
            if (i > line_len || line[i] == '\0') {
                // End of line reached
                return len;
            } else {
                // Comma found, skip it
                i++;
            }
        } else {
            // Skip current field
            int in_quotes = 0;
            if (line[i] == '"') {
                in_quotes = 1;
                i++;
            }
            
            while (i < line_len) {
                if (in_quotes) {
                    if (line[i] == '"') {
                        if (i + 1 < line_len && line[i+1] == '"') {
                            i += 2;
                        } else {
                            in_quotes = 0;
                            i++;
                            break;
                        }
                    } else {
                        i++;
                    }
                } else {
                    if (line[i] == ',') {
                        break;
                    }
                    i++;
                }
            }
            
            if (i >= line_len || line[i] == '\0') {
                // End of line reached before finding target index
                return -1;
            }
            
            i++; // Skip the comma
            field_count++;
        }
    }

    // If we finished the loop without returning, idx was out of range
    return -1;
}