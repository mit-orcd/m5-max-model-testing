csv_to_tsv() {
    local file="$1"
    if [ ! -f "$file" ]; then
        return 1
    fi

    awk '
    BEGIN {
        FS = "\n"
        OFS = "\n"
        in_quotes = 0
        current_field = ""
        current_record = ""
        field_count = 0
        buffer = ""
    }

    function process_char(c) {
        if (in_quotes) {
            if (c == "\"") {
                # Check if next char is also a quote (escaped quote)
                # We need to look ahead, but awk is line-oriented
                # This approach is tricky. Let us restart with a state machine.
                # Actually, let us rewrite to handle character by character if possible,
                # or use a robust line-based parser that handles quotes.
            }
        }
    }

    {
        # Remove trailing CR
        gsub(/\r$/, "")
        
        line = $0
        
        # Append to buffer if we are in a quoted field (from previous lines)
        if (in_quotes) {
            line = buffer line
            buffer = ""
        }
        
        n = length(line)
        i = 1
        field_str = ""
        field_count = 0
        out_record = ""
        output_fields = ""
        
        # State: 0 = normal, 1 = in quotes
        state = 0
        
        # We will parse the line into fields, handling quotes manually.
        # However, since the input can span multiple lines, we need to accumulate.
        # Let us change strategy: read the whole file into a string and parse.
        # But for large files, that is bad.
        # Let us do a robust character-by-character parse using a loop.
        
        # Reset for new record
        delete fields
        field_count = 0
        in_quotes = 0
        current_field = ""
        continuation = 0 # 1 if previous line ended with un-closed quote
        
        if (continuation == 1) {
            line = accumulated_line line
            accumulated_line = ""
        }
        
        len = length(line)
        i = 1
        current_field = ""
        field_count = 0
        in_quotes = 0
        last_char = ""
        
        while (i <= len) {
            c = substr(line, i, 1)
            
            if (in_quotes) {
                if (c == "\"") {
                    # Could be end of field or escaped quote
                    # Look ahead
                    if (i < len) {
                        next_c = substr(line, i+1, 1)
                        if (next_c == "\"") {
                            # Escaped quote
                            current_field = current_field "\""
                            i++ # Skip next quote
                        } else if (next_c == ",") {
                            # End of field
                            in_quotes = 0
                            field_count++
                            fields[field_count] = current_field
                            current_field = ""
                            i++ # Skip comma
                            # Check if field ended with newline (CR/LF handled by line splitting?)
                            # No, newlines inside quotes are preserved as literal \n
                            # But wait, if in_quotes, we might have hit a newline in the raw input?
                            # No, awk splits by newline. So if we are in_quotes, this line
                            # is a continuation of a quoted field from the previous line.
                            # We should NOT split on newline here.
                            # So, if in_quotes, we just append the line to the previous one.
                        } else {
                            # End of field (normal)
                            in_quotes = 0
                            field_count++
                            fields[field_count] = current_field
                            current_field = ""
                            # Next char is not comma, so it might be end of record or garbage?
                            # RFC 4180 says fields are separated by commas.
                            # If not comma, and not end of string, it is invalid?
                            # We will just treat it as end of field.
                        }
                    } else {
                        # End of line while in quotes
                        # This means the quote was not closed on this line.
                        # The field continues to the next line.
                        current_field = current_field "\""
                        # Do not close field yet.
                        # Mark that we need to continue reading.
                        continuation = 1
                        accumulated_line = "" # Already prepended
                    }
                } else {
                    current_field = current_field c
                }
            } else {
                if (c == "\"") {
                    # Start of quoted field? Or just a quote?
                    # RFC 4180: If field is quoted, it MUST start with quote.
                    # If it starts with quote, it must end with quote.
                    if (current_field == "") {
                        in_quotes = 1
                    } else {
                        # Quote inside unquoted field? Invalid or literal?
                        # We will treat it as literal quote.
                        current_field = current_field c
                    }
                } else if (c == ",") {
                    field_count++
                    fields[field_count] = current_field
                    current_field = ""
                    # Check if last char was a quote? No, in_quotes is 0.
                } else if (c == "\n" || c == "\r") {
                    # Should not happen here because awk splits lines.
                    # But if we are in_quotes, we shouldn't be splitting.
                    # This logic assumes we prepend previous lines if in_quotes.
                } else {
                    current_field = current_field c
                }
            }
            i++
        }
        
        # Process the last field
        if (in_quotes) {
            # If we ended in_quotes, it means the quote was not closed.
            # We treat the field as is, but the record is incomplete.
            # RFC 4180 allows this? No, but we must handle it.
            # The field continues to next line.
            field_count++
            fields[field_count] = current_field
            continuation = 1
            accumulated_line = ""
        } else {
            field_count++
            fields[field_count] = current_field
            continuation = 0
        }
        
        # If continuation is 0, we can output the record.
        if (continuation == 0) {
            # Output the record
            out_line = ""
            for (j = 1; j <= field_count; j++) {
                f = fields[j]
                # Replace literal tabs and newlines in the field with escaped sequences
                # Note: Since awk splits by newline, a newline inside a quoted field
                # would have caused this line split.
                # So, if we are processing a multi-line quoted field, we have accumulated it.
                # Does the accumulated string contain actual newlines?
                # No, because we removed the newline when we prepended?
                # Let's check the accumulation logic.
                # When we do `line = accumulated_line line`, we are concatenating strings.
                # The newline that separated the lines is lost.
                # We need to preserve the newline as a literal \n in the output.
                
                # Let's refine the accumulation:
                # If a line ends with an unclosed quote, we set `continuation = 1`.
                # We need to store the newline that was skipped.
                
                # This simple awk approach is flawed for multi-line fields.
                # Let's rewrite using a full buffer approach.
            }
        }
    }
    ' "$file"
}