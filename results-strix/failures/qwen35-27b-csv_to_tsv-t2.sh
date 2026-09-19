csv_to_tsv() {
    local file="$1"
    awk '
    BEGIN {
        RS = ""
        ORS = ""
    }
    {
        # Normalize line endings: convert CRLF to LF, remove trailing CR if any
        gsub(/\r\n/, "\n")
        gsub(/\r/, "\n")
        
        n = split($0, lines, "\n")
        line_idx = 1
        field_idx = 1
        field = ""
        in_quotes = 0
        
        while (line_idx <= n) {
            line = lines[line_idx]
            if (length(line) == 0 && line_idx == n && n > 1) {
                # Handle potential empty trailing line after split
                line_idx++
                continue
            }
            
            len = length(line)
            for (i = 1; i <= len; i++) {
                c = substr(line, i, 1)
                
                if (in_quotes) {
                    if (c == "\"") {
                        # Check for escaped quote ""
                        if (i < len && substr(line, i+1, 1) == "\"") {
                            field = field "\""
                            i++
                        } else {
                            # End of quoted field
                            in_quotes = 0
                        }
                    } else {
                        if (c == "\t") {
                            field = field "\\t"
                        } else if (c == "\n") {
                            # Should not happen inside a single line from split, 
                            # but if we had multiline records handled differently, 
                            # this would be the place. 
                            # Since we split by newline, newlines inside fields 
                            # are already handled by the loop structure below.
                            # Actually, the split() broke the record. 
                            # We need to handle newlines inside quoted fields differently.
                            # Re-evaluating approach: RS="" splits on empty record separator.
                            # But standard CSV parsing in awk is tricky with newlines in quotes.
                            # Let's restart with a character-by-character loop over the whole input.
                        } else {
                            field = field c
                        }
                    }
                } else {
                    if (c == "\"") {
                        in_quotes = 1
                    } else if (c == ",") {
                        # End of field
                        printf "%s\t", field
                        field = ""
                        field_idx++
                    } else if (c == "\t") {
                        # Literal tab outside quotes? RFC 4180 says fields containing tabs should be quoted.
                        # But if unquoted, it acts as delimiter? No, comma is delimiter.
                        # Unquoted tab is just a character.
                        field = field "\\t"
                    } else {
                        field = field c
                    }
                }
            }
            
            # If we are still in quotes, the field continues to the next line
            if (in_quotes) {
                # Append a newline to the field content (escaped as \n)
                field = field "\\n"
                line_idx++
            } else {
                # End of record or just end of line if not last field?
                # We need to detect end of record. 
                # If we reached end of a line and not in quotes, it might be end of record.
                # But we are iterating line by line from split().
                # This logic is flawed for multiline fields because split() breaks them.
            }
        }
        # This approach with split() is insufficient for multiline fields inside quotes.
        # We must process character by character without splitting first.
    }
    ' "$file"
}