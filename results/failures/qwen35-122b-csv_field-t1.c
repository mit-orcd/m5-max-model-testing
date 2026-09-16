```c
#include <stddef.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (line == NULL || out == NULL || cap == 0) {
        return -1;
    }

    const char *p = line;
    int current_idx = 0;
    size_t out_len = 0;
    int in_quotes = 0;

    while (*p != '\0' && *p != '\n' && *p != '\r') {
        if (current_idx == idx) {
            // We are extracting the target field
            if (*p == '"') {
                in_quotes = 1;
                p++;
                continue;
            }

            while (*p != '\0' && *p != '\n' && *p != '\r') {
                if (*p == ',' && !in_quotes) {
                    break;
                }

                if (*p == '"') {
                    if (in_quotes) {
                        // Check for escaped quote ""
                        if (*(p + 1) == '"') {
                            if (out_len < cap - 1) {
                                out[out_len++] = '"';
                            }
                            p += 2;
                            continue;
                        } else {
                            // End of quoted section
                            in_quotes = 0;
                            p++;
                            continue;
                        }
                    } else {
                        // Quote at start of unquoted field? RFC 4180 implies quoted fields start with "
                        // But if we are in the middle of an unquoted field and see a quote, it's invalid or literal?
                        // RFC 4180: A field enclosed in double quotes...
                        // Standard parsers treat a quote in an unquoted field as a literal or error.
                        // We will treat it as a literal character if not at the very start (handled by in_quotes logic above).
                        // However, the logic above handles the start. If we hit a quote here without in_quotes,
                        // it's ambiguous. Standard behavior: if not in quotes, a quote is just a char?
                        // Actually, RFC 4180 says: "Fields containing line breaks, double quotes, and commas should be enclosed in double-quotes."
                        // It doesn't explicitly forbid quotes in unquoted fields, but implies they are delimiters.
                        // To be safe and robust: If we see a quote and not in_quotes, we treat it as the start of a quoted section?
                        // No, the parser usually checks for quote at the very start of the field.
                        // Let's stick to: if we are not in_quotes, and we see a quote, it's likely an error or literal.
                        // But our logic: if current_idx == idx, and we see a quote at the start, we set in_quotes=1.
                        // If we are in the middle of an unquoted field (which shouldn't happen with a quote), we just add it.
                        // Re-evaluating: The loop logic for "in_quotes" is set at the very first char of the field.
                        // If we are here, we are inside the field body.
                        // If *p == '"' and !in_quotes: This implies a quote appeared in the middle of an unquoted field.
                        // RFC 4180 doesn't strictly define this, but most parsers treat it as a literal.
                        if (out_len < cap - 1) {
                            out[out_len++] = *p;
                        }
                        p++;
                        continue;
                    }
                }

                if (out_len < cap - 1) {
                    out[out_len++] = *p;
                }
                p++;
            }
            break;
        } else {
            // Skipping current field
            if (*p == '"') {
                in_quotes = 1;
                p++;
                while (*p != '\0' && *p != '\n' && *p != '\r') {
                    if (*p == '"') {
                        if (*(p + 1) == '"') {
                            p += 2;
                            continue;
                        } else {
                            in_quotes = 0;
                            p++;
                            break;
                        }
                    }
                    p++;
                }
            } else {
                while (*p != '\0' && *p != '\n' && *p != '\r' && *p != ',') {
                    p++;
                }
            }
            if (*p == ',') {
                p++;
                current_idx++;
            } else if (*p ==