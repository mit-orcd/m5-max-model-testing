csv_to_tsv() {
    local file="$1"
    if [[ -z "$file" || ! -f "$file" ]]; then
        return 1
    fi

    awk '
    BEGIN {
        RS = ""  # Not used directly, we handle parsing manually
        # We will read character by character effectively via line processing
        # But awk line-based is tricky with embedded newlines.
        # Instead, we read the whole file into a string or process carefully.
        # A robust way in awk for RFC4180 is to read the entire file into a single string
        # and then parse it.
    }
    {
        # Accumulate all lines into one big string
        if (NR == 1) {
            content = $0
        } else {
            content = content "\n" $0
        }
    }
    END {
        # Remove trailing CR if present (handle CRLF)
        gsub(/\r$/, "", content)
        
        len = length(content)
        pos = 1
        first_record = 1
        
        while (pos <= len) {
            # Start of a new record
            if (!first_record) {
                printf "\n"
            }
            first_record = 0
            
            # Parse fields until end of record
            first_field = 1
            while (pos <= len) {
                if (!first_field) {
                    printf "\t"
                }
                first_field = 0
                
                # Check if field starts with quote
                ch = substr(content, pos, 1)
                if (ch == "\"") {
                    # Quoted field
                    pos++ # skip opening quote
                    field_val = ""
                    while (pos <= len) {
                        ch = substr(content, pos, 1)
                        if (ch == "\"") {
                            # Check next char
                            next_ch = substr(content, pos + 1, 1)
                            if (next_ch == "\"") {
                                # Escaped quote
                                field_val = field_val "\""
                                pos += 2
                            } else {
                                # End of quoted field
                                pos++ # skip closing quote
                                break
                            }
                        } else {
                            # Handle special characters for TSV output
                            if (ch == "\n") {
                                field_val = field_val "\\n"
                            } else if (ch == "\t") {
                                field_val = field_val "\\t"
                            } else {
                                field_val = field_val ch
                            }
                            pos++
                        }
                    }
                    # After closing quote, expect comma or end of record (newline or EOF)
                    # Skip any trailing whitespace? No, RFC4180 is strict.
                    # Check for comma or newline or EOF
                    if (pos <= len) {
                        ch = substr(content, pos, 1)
                        if (ch == ",") {
                            pos++ # skip comma, continue to next field
                            continue
                        } else if (ch == "\n") {
                            pos++ # skip newline, end of record
                            break
                        } else {
                            # Unexpected character, treat as end of record? 
                            # Or error? Let us assume well-formed or just break.
                            break
                        }
                    } else {
                        # EOF
                        break
                    }
                } else {
                    # Unquoted field
                    field_val = ""
                    while (pos <= len) {
                        ch = substr(content, pos, 1)
                        if (ch == ",") {
                            pos++ # skip comma
                            break
                        } else if (ch == "\n") {
                            pos++ # skip newline
                            break
                        } else {
                            field_val = field_val ch
                            pos++
                        }
                    }
                    # If we broke due to comma, continue to next field
                    # If we broke due to newline or EOF, end record
                    if (pos <= len) {
                        ch = substr(content, pos, 1)
                        if (ch == ",") {
                            # Already skipped comma above? No, the break happened after checking.
                            # Wait, in the loop: if ch==",", pos++, break.
                            # So pos is now after comma.
                            continue
                        } else if (ch == "\n") {
                            # Already skipped newline above? No.
                            # In loop: if ch=="\n", pos++, break.
                            # So pos is now after newline.
                            break
                        }
                    } else {
                        # EOF
                        break
                    }
                }
            }
            # End of record
        }
        # Ensure final newline
        printf "\n"
    }
    ' "$file"
}