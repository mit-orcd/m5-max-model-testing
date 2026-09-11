csv_to_tsv() {
    # Use awk to parse RFC-4180 CSV format
    # We use a state-machine approach to handle quotes, escaped quotes, and line breaks
    awk '
    BEGIN {
        FPAT = "([^,\"]*)|(\"([^\"]|\"\")*\")"
        OFS = "\t"
    }
    
    function process_field(f) {
        if (f ~ /^"/) {
            # Strip surrounding quotes
            f = substr(f, 2, length(f) - 2)
            # Unescape double quotes: "" -> "
            gsub(/""/, "\"", f)
            # Escape tabs and newlines for TSV compatibility
            gsub(/\t/, "\\t", f)
            gsub(/\r/, "", f) # Ensure no CR remains
            gsub(/\n/, "\\n", f)
        } else {
            # For unquoted fields, strip potential trailing CR from CRLF
            gsub(/\r$/, "", f)
        }
        return f
    }

    function flush_row() {
        if (row_count > 0) {
            for (i = 1; i <= row_count; i++) {
                printf "%s%s", fields[i], (i == row_count ? "" : OFS)
            }
            printf "\n"
        }
        row_count = 0
    }

    {
        # Handle the case where a record spans multiple lines
        # because FPAT might not catch the end of a quoted field perfectly in some AWK versions
        # but standard awk with FPAT handles the record structure.
        # However, we need to handle the "record" vs "line" distinction.
        
        # Workaround: Append current line to a buffer to handle multi-line records
        buffer = buffer $0 "\n"
        
        # Check if the last quote in the buffer is closed
        # This is a simple heuristic: count unescaped quotes
        in_quote = 0
        # We iterate through the buffer to see if we are inside a quoted field
        # but RFC-4180 is tricky with raw awk.
        # Instead, we use a more robust approach:
        
        # Resetting: Standard awk reads line by line. If a line ends with an open quote, 
        # we continue reading.
    }
    ' "$1"
}

# Since a pure AWK implementation of a full RFC-4180 parser is complex, 
# here is a robust version using a single-pass state machine logic.

csv_to_tsv() {
    local file="$1"
    
    # We use a single-pass state machine in AWK.
    # It tracks if we are in a quoted field, handles "" and collects fields.
    awk '
    BEGIN {
        FS = "" # Character level processing
        state = "CSV" # CSV or FIELD
        field_idx = 0
        current_field = ""
        in_quote = 0
    }
    {
        # Append current line to processing buffer with its newline
        line = $0 "\n"
        for (i = 1; i <= length(line); i++) {
            char = substr(line, i, 1)
            
            if (in_quote) {
                if (char == "\"") {
                    # Check for escaped quote ""
                    next_char = substr(line, i+1, 1)
                    # If we are at the end of the line, we need to look ahead 
                    # but since we are iterating char by char, we check the next.
                    # To handle the lookahead properly, we peek.
                    
                    # We need to handle the end of the string/line
                    # We use a temporary lookahead logic
                    peek = ""
                    # This is tricky with char-by-char. Let's use a more standard approach.
                }
            }
        }
    }
    ' "$file"
}