```c
#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    int field_idx = 0;
    const char *p = line;
    size_t out_len = 0;
    int in_quotes = 0;
    int found = 0;
    size_t total_len = 0;

    if (idx < 0 || cap == 0) {
        if (cap == 0) return -1;
        *out = '\0';
        return -1;
    }

    while (*p != '\0' && *p != '\n' && *p != '\r') {
        if (field_idx == idx) {
            found = 1;
        }

        if (field_idx == idx && out_len < cap - 1) {
            if (!in_quotes && *p == ',') {
                break;
            }
            if (*p == '"') {
                if (!in_quotes) {
                    in_quotes = 1;
                    p++;
                    continue;
                } else {
                    if (*(p + 1) == '"') {
                        if (out_len < cap - 1) {
                            out[out_len++] = '"';
                        }
                        p += 2;
                        total_len++;
                        continue;
                    } else {
                        in_quotes = 0;
                        p++;
                        continue;
                    }
                }
            }
            out[out_len++] = *p++;
            total_len++;
        } else {
            if (*p == '"') {
                if (!in_quotes) {
                    in_quotes = 1;
                } else {
                    if (*(p + 1) == '"') {
                        p += 2;
                        continue;
                    }
                    in_quotes = 0;
                }
            } else if (!in_quotes && *p == ',') {
                field_idx++;
            }
            p++;
            if (field_idx > idx) break;
        }
    }

    if (!found) {
        if (field_idx > idx) return -1;
        if (field_idx == idx) {
            // Handle last field if loop ended due to EOF
            found = 1;
        } else {
            return -1;
        }
    }

    if (found) {
        // Recalculate total unescaped length if we didn't count it fully
        if (cap > 1) {
            out[out_len] = '\0';
        } else {
            out[0] = '\0';
        }
        
        // We need the true unescaped length regardless of cap.
        // Re-scan or track properly. Let's track total_len correctly from start.
        // The previous logic mixed tracking. Let's rewrite the counting logic cleanly.
    }

    // Reset and do a proper pass for length calculation to ensure correctness
    // Actually, let's integrate the length calculation properly in one pass.
    
    // Restart logic for clarity and correctness
    p = line;
    field_idx = 0;
    in_quotes = 0;
    out_len = 0;
    total_len = 0;
    int current_field_idx = 0;
    int target_found = 0;

    if (idx < 0) return -1;

    while (*p != '\0' && *p != '\n' && *p != '\r') {
        if (current_field_idx == idx) {
            target_found = 1;
        }

        if (*p == '"') {
            if (!in_quotes) {
                in_quotes = 1;
                p++;
                continue;
            } else {
                if (*(p + 1) == '"') {
                    total_len++;
                    if (target_found && out_len < cap - 1) {
                        out[out_len++] = '"';
                    }
                    p += 2;
                    continue;
                } else {
                    in_quotes = 0;
                    p++;
                    continue;
                }
            }
        }

        if (*p == ',' && !in_quotes) {
            if (current_field_idx == idx) {
                // End of target field
                break;
            }
            current_field_idx++;
            p++;
            continue;
        }

        total_len++;
        if (target_found && out_len < cap - 1) {
            out[out_len++] = *p;
        }
        p++;
    }