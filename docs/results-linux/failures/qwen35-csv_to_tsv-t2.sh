csv_to_tsv() {
    local file="$1"
    awk -v RS='(\r?\n|\r)' -v ORS='' '
    BEGIN {
        FS = ""
    }
    {
        # Normalize line endings: remove trailing CR if present
        sub(/\r$/, "")
        
        # If line is empty and we are not in a quoted field, skip
        if (length($0) == 0 && !in_quotes) {
            next
        }
        
        # Accumulate lines if we are inside a quoted field
        if (in_quotes) {
            buffer = buffer "\n" $0
        } else {
            buffer = $0
        }
        
        # Process the accumulated buffer if we are at the end of a record
        # A record ends when we are not inside quotes
        if (!in_quotes) {
            parse_record(buffer)
            buffer = ""
        }
    }
    END {
        if (buffer != "") {
            parse_record(buffer)
        }
    }
    
    function parse_record(line,    i, c, in_q, field, len, out, j, char, next_char) {
        len = length(line)
        in_q = 0
        field = ""
        out = ""
        
        for (i = 1; i <= len; i++) {
            c = substr(line, i, 1)
            
            if (!in_q) {
                if (c == "\"") {
                    in_q = 1
                    field = ""
                } else if (c == ",") {
                    # Output the field
                    if (out != "") out = out "\t"
                    out = out field
                    field = ""
                } else if (c == "\n" || c == "\r") {
                    # End of record (should not happen inside !in_q unless empty line)
                    # But if we are here, it means the line ended without a quote
                    if (out != "") out = out "\t"
                    out = out field
                    print out
                    return
                } else {
                    field = field c
                }
            } else {
                # Inside quotes
                if (c == "\"") {
                    next_char = (i < len) ? substr(line, i+1, 1) : ""
                    if (next_char == "\"") {
                        # Escaped quote
                        field = field "\""
                        i++ # Skip next char
                    } else {
                        # End of quoted field
                        in_q = 0
                    }
                } else if (c == "\n" || c == "\r") {
                    # Line break inside quoted field
                    field = field "\n"
                } else {
                    field = field c
                }
            }
        }
        
        # Output the last field of the record
        if (out != "") out = out "\t"
        out = out field
        print out
    }
    ' "$file"
}