```c
#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || idx < 0) {
        return -1;
    }

    int field_index = 0;
    const char *p = line;
    size_t out_len = 0;
    int found = 0;

    while (1) {
        // Check if we are at the start of the target field
        if (field_index == idx) {
            found = 1;
            // Now parse this field
            if (*p == '"') {
                // Quoted field
                p++; // skip opening quote
                while (*p != '\0') {
                    if (*p == '"') {
                        if (*(p + 1) == '"') {
                            // Escaped quote
                            if (out_len < cap - 1) {
                                out[out_len] = '"';
                            }
                            out_len++;
                            p += 2;
                        } else {
                            // End of quoted field
                            p++; // skip closing quote
                            // After closing quote, expect comma, newline, or end of string
                            if (*p == ',') {
                                p++; // skip comma for next field
                                field_index++;
                                // Continue outer loop to check next field
                                break;
                            } else if (*p == '\0') {
                                // End of line
                                break;
                            } else {
                                // Malformed, but we treat remaining chars as part of field or ignore?
                                // RFC 4180 says after closing quote should be comma or end.
                                // Let's just break out of quoted parsing.
                                // Actually, if there's extra chars, they might be appended in some parsers.
                                // For strict RFC, this is invalid. But let's assume well-formed or handle gracefully.
                                // We'll break out of quoted parsing here.
                                break;
                            }
                        }
                    } else {
                        if (out_len < cap - 1) {
                            out[out_len] = *p;
                        }
                        out_len++;
                        p++;
                    }
                }
                // If we exited the quoted field loop because *p == '\0', we are done.
                if (*p == '\0') {
                    break;
                }
                // If we broke out due to comma after closing quote, field_index was incremented.
                // The outer while loop continues.
                continue;
            } else {
                // Unquoted field
                while (*p != ',' && *p != '\0') {
                    if (out_len < cap - 1) {
                        out[out_len] = *p;
                    }
                    out_len++;
                    p++;
                }
                if (*p == ',') {
                    p++; // skip comma
                    field_index++;
                    continue;
                } else {
                    // *p == '\0'
                    break;
                }
            }
        } else {
            // Skip this field
            if (*p == '"') {
                // Quoted field
                p++; // skip opening quote
                while (*p != '\0') {
                    if (*p == '"') {
                        if (*(p + 1) == '"') {
                            p += 2;
                        } else {
                            // End of quoted field
                            p++; // skip closing quote
                            if (*p == ',') {
                                p++; // skip comma
                                field_index++;
                                break;
                            } else {
                                // End of line or malformed
                                // If *p is '\0' or other, we've reached end of line after quoted field
                                // Set p to point past end to break outer loop eventually
                                // But we need to check if we're at end of line
                                if (*p == '\0') {
                                    // This is the last field
                                    // field_index doesn't matter anymore
                                    p = NULL; // signal break
                                    break;
                                } else {
                                    // Malformed, skip rest
                                    p = NULL;
                                    break;
                                }
                            }
                        }
                    } else {
                        p++;
                    }
                }
                if (p == NULL) {
                    break;
                }
                // If we broke out of inner while because of comma, field_index was incremented.
                // Continue outer loop.
                continue;
            } else {
                // Unquoted field
                while (*p != ',' && *p != '\0') {