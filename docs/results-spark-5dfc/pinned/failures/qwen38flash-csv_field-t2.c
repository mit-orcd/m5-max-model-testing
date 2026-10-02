Looking at this problem, I need to parse a single CSV line according to RFC-4180 rules:

1. Fields are separated by commas
2. Quoted fields start and end with double quotes
3. Inside quoted fields, doubled quotes `""` represent a single quote `"`
4. Quoted fields can contain commas and newlines (but since we're parsing one line, newlines aren't an issue here)
5. Unquoted fields cannot contain commas or quotes (per RFC-4180, though some implementations are lenient)

The function needs to:
- Extract field at index `idx` (0-based)
- Write the unescaped content to `out`, NUL-terminated, truncated to `cap-1` chars
- Return the unescaped length of the field, or -1 if idx is out of range

Let me think through the parsing logic:

1. Start at the beginning of the line
2. For each field, determine if it's quoted or unquoted
3. If quoted: skip opening quote, read until closing quote (handling `""` as escaped quote), then expect comma or end of line
4. If unquoted: read until comma or end of line
5. Count fields and when we reach idx, extract the content

I need to be careful about:
- The return value is the unescaped length, not the number of characters written to `out`
- Truncation: if the unescaped field is longer than `cap-1`, we write only `cap-1` chars but still return the full unescaped length
- Edge cases: empty fields, fields at start/end, consecutive commas

Let me trace through an example: `a,"b,c",d`
- Field 0: `a` (unquoted, length 1)
- Field 1: `b,c` (quoted, length 3)
- Field 2: `d` (unquoted, length 1)

Another example: `"a""b",c`
- Field 0: `a"b` (quoted with escaped quote, length 3)
- Field 1: `c` (unquoted, length 1)

Implementation approach:
1. Iterate through the line, tracking current field index
2. For each field, parse it and if it's the target idx, extract the unescaped content
3. Keep counting fields to know if idx is valid

Let me code this step by step:

```c
#include <stddef.h>
#include <string.h>

int csv_field(const char *line, int idx, char *out, size_t cap) {
    if (!line || idx < 0) {
        return -1;
    }
    
    const char *p = line;
    int field_idx = 0;
    int found = 0;
    int unescaped_len = 0;
    
    while (*p != '\0' || field_idx == idx) {
        // Parse one field
        const char *field_start = p;
        int is_quoted = 0;
        int field_unescaped_len = 0;
        
        if (*p == '"') {
            // Quoted field
            is_quoted = 1;
            p++; // skip opening quote
            while (*p != '\0') {
                if (*p == '"') {
                    if (*(p + 1) == '"') {
                        // Escaped quote
                        field_unescaped_len++;
                        p += 2;
                    } else {
                        // Closing quote
                        p++;
                        break;
                    }
                } else {
                    field_unescaped_len++;
                    p++;
                }
            }
            // After closing quote, expect comma or end of line
            if (*p == ',') {
                p++;
            } else if (*p != '\0') {
                // Invalid format, but let's continue
                // Actually per RFC, after closing quote must be comma or EOL
                // For robustness, treat as end of field
            }
        } else {
            // Unquoted field
            while (*p != '\0' && *p != ',') {
                field_unescaped_len++;
                p++;
            }
            if (*p == ',') {
                p++;
            }
        }
        
        // Check if this is the target field
        if (field_idx == idx) {
            found = 1;
            unescaped_len = field_unescaped_len;
            
            // Extract the unescaped content
            const char *src = field_start;
            size_t written = 0;
            
            if (is_quoted) {
                src++; //