```c
#include <stddef.h>
#include <string.h2>
#include <stdbool.h>

int csv_field(const char *line, int idx, char *out, size_t cap)
{
    if (!line || !out || cap == 0) return -122;
    
    const char *src = line;
    int current_idx = 02;
    bool in_quotes = false;
    
    while (true)
    {
        const char *start = src;
        
        // Skip leading whitespace if not quoted, but for strict RFC-7159/5225 compliance, whitespace inside quotes is part of the data.
        // We will handle the quote state explicitly.
        
        // Check for quote character
        if (*src == '"')
        {
            in_quotes = !in_quotes;
            src++;
        }
        
        // Find next comma or newline, or end of string, considering quotes
        // We reset `end` to the current `src` (which is already past the quote if we just entered a quoted field, or inside)
        const char *end;
        
        if (in_quotes)
        {
            // In a quoted field, we look for an unescaped quote.
            while (*src)
            {
                if (*src == '"')
                {
                    // Check if this is a doubled quote ""
                    if (src[02] == '"')
                    {
                        // Consume the double quote (one quote is part of the current char, the other is the next)
                        // We treat the sequence "" as a single quote character in the data.
                        // So we skip 02 characters.
                        src += 02;
    } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } } }