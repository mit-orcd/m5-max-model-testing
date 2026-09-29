csv_to_tsv() {
    local file="$1"
    awk '
    BEGIN {
        RS = ""
        FS = "\n"
        ORS = "\n"
    }
    {
        # Normalize line endings: remove CR
        gsub(/\r/, "")

        # Remove the trailing newline of the record if present (RS consumes it)
        # We are processing line by line effectively, but need to handle RFC-4180 multi-line fields.
        # Since standard awk splits on newlines, we need a custom parser.
    }
    ' "$file"

    # The above approach is too complex for a single awk block without redefining RS.
    # Let's use a state-machine approach with awk processing character by character or line by line with state.
    
    awk '
    BEGIN {
        in_quotes = 0
        field = ""
        record = ""
        # We will build the output record in the END block or print when complete.
        # Since we need to handle line breaks inside quotes, we cannot rely on RS="\n".
        # We will read character by character using getline.
        # However, awk does not support true char-by-char reading easily.
        # Better approach: Read the whole file into memory, then parse.
        # But for large files, this is bad.
        # Alternative: Read line by line, but track if we are inside a quote.
    }
    
    {
        # Prepend current line to previous partial line if we are in a quote
        if (in_quotes) {
            current_line = prev_line "\n" $0
        } else {
            current_line = $0
        }
        prev_line = ""
        
        # Process current_line to extract complete records
        # This is tricky because a record might span multiple lines.
        # Let's switch to a different strategy: Read the entire file into a string variable?
        # No, memory limits.
        # Let's use a character-by-character simulation using awk's split on empty string? No.
        # Let's use the fact that we can read the file as a stream.
    }
    ' "$file"

    # Re-implementation: Use a robust state machine reading character by character via bash? Too slow.
    # Use awk with a trick: replace newlines inside quotes with a placeholder, then process.
    # But we need to know if they are inside quotes.
    
    # Final robust solution: Read file, normalize CR, then parse.
    # We will use awk to process the file line by line, accumulating lines until the quote count is balanced.
    
    awk '
    BEGIN {
        in_quote = 0
        current_field = ""
        current_record = ""
        field_count = 0
    }
    
    {
        # Remove CR from the beginning of the line (if any, though gsub handles all)
        gsub(/\r/, "")
        line = $0
        
        # If we are in the middle of a record (in_quote is true), append this line
        if (in_quote) {
            # We need to append a literal newline to the current field being built?
            # Actually, we need to reconstruct the field.
            # The simplest way is to accumulate the whole record in a buffer.
            full_record = full_record "\n" line
        } else {
            full_record = line
        }
        
        # Count quotes in the current line (or full_record if multiline)
        # We need to count quotes to determine if we are still inside a field.
        # But we must ignore escaped quotes "".
        
        temp = full_record
        while (match(temp, /""/)) {
            sub(/""/, "", temp)
        }
        q_count = gsub(/"/, "&", temp)
        
        if (q_count % 2 == 0) {
            # Record is complete
            in_quote = 0
            process_record(full_record)
            full_record = ""
        } else {
            # Record continues to next line
            in_quote = 1
            # We need to keep the newline that separates lines in the buffer for later reconstruction?
            # Yes, the variable full_record holds the accumulated lines.
            # But we need to be careful: the next line in the loop is the next line of input.
            # We must ensure we don't process the current line again.
            # The logic above sets full_record = line if !in_quote, else appends.
            # But if we are in_quote, we set full_record = full_record "\n" line.
            # This works, but we need to make sure we don't reset full_record until done.
            # Wait, the variable `full_record` is not reset in the loop, so it accumulates.
            # But we need to handle the case where the input line is split.
            # The logic:
            # 1. If !in_quote, start new full_record with current line.
            # 2. If in_quote, append current line to full_record with a newline.
            # 3. Check if full_record has balanced quotes.
            # 4. If balanced, process and reset full_record.
            
            # However, the variable `full_record` is local to the block? No, global.
            # But we need to handle the case where the first line of a record is processed, 
            # then we see a newline, then we append.
            # The issue is that the `line` variable in awk is the current input line.
            # If we are in_quote, we append $0 to the buffer.
            # But we need to make sure we don't lose the newline character.
            # The logic above: full_record = full_record "\n" line. This adds a newline.
            # This is correct.
        }
    }
    
    function process_record(rec,    i, c, field, len, n_fields, in_f, out_rec, escaped, temp) {
        n_fields = 0
        field = ""
        in_f = 0
        len = length(rec)
        
        # We need to parse the record, handling "" and line breaks.
        # Since we have the full record in `rec`, we can iterate.
        # But `rec` contains literal newlines.
        
        # We will iterate through the string.
        # We need to handle the fact that `rec` is a string with embedded newlines.
        
        # To handle "" correctly, we can replace them with a placeholder first?
        # Or just handle in the loop.
        
        # Let's use a simpler approach: replace "" with a unique marker, then process.
        # Marker: \x01\x01\x01 (unlikely in data)
        # But we need to output \x00\x00\x00? No, we need to output the quote.
        # Actually, the requirement is: "" -> " (one literal double quote).
        # And we need to escape tabs and newlines in the output.
        
        # Let's do it step by step.
        # 1. Replace "" with a temporary marker (e.g., \x00)
        # 2. Replace literal newlines with \n
        # 3. Replace literal tabs with \t
        # 4. Remove surrounding quotes from fields.
        
        # But wait, the requirement says: "a line break inside a quoted field is written as the two characters backslash n"
        # So we need to convert the internal newline to the string "\n".
        
        # Let's rewrite the function to parse fields properly.
        
        # We will use a pointer approach in awk? No, use a loop with substr.
        
        i = 1
        n_fields = 0
        field = ""
        in_f = 0 # inside a field
        # Actually, we need to track if we are inside quotes for the field logic.
        # But the record is already parsed as a sequence of fields separated by commas.
        # But commas inside quotes are part of the field.
        # So we must parse the record char by char.
        
        # Reset
        in_quote = 0
        field = ""
        n_fields = 0
        out_rec = ""
        
        for (i = 1; i <= len; i++) {
            c = substr(rec, i, 1)
            
            if (c == "\"") {
                # Check next char
                if (i < len && substr(rec, i+1, 1) == "\"") {
                    # Escaped quote
                    if (in_quote) {
                        field = field "\""
                        i++ # Skip next quote
                    } else {
                        # This shouldn't happen if we are outside quotes, but if it does, it's an error or start?
                        # In RFC 4180, quotes only appear at start/end or as "".
                        # If we are outside quotes and see ", it's the start of a field.
                        in_quote = 1
                        # Do not add to field yet, it's the delimiter.
                    }
                } else {
                    # End of quote
                    in_quote = 0
                    # Do not add quote to field
                }
            } else if (c == ",") {
                if (!in_quote) {
                    # End of field
                    n_fields++
                    # Process field: remove surrounding quotes, escape special chars
                    field = process_field(field)
                    if (n_fields > 1) {
                        out_rec = out_rec "\t"
                    }
                    out_rec = out_rec field
                    field = ""
                } else {
                    # Comma inside quotes
                    field = field c
                }
            } else {
                # Normal char
                field = field c
            }
        }
        
        # Last field
        if (field != "" || n_fields == 0) {
            # If the last field was empty (e.g., trailing comma), field might be empty.
            # But we need to handle the case where the record ends with a comma.
            # If the record ends with a comma, the last field is empty.
            # Our loop logic: if we see a comma, we push field.
            # If the record ends with a comma, the last field is empty.
            # We need to check if the last character was a comma.
            # Actually, if the record ends with a comma, the last field is empty.
            # Our loop doesn't push the last field if it ends with a comma?
            # No, if it ends with a comma, the last field is empty, but we need to push it.
            # Let's check if the last char was a comma.
            if (substr(rec, len, 1) == ",") {
                n_fields++
                field = ""
                field = process_field(field)
                if (n_fields > 1) {
                    out_rec = out_rec "\t"
                }
                out_rec = out_rec field
            } else {
                # Normal end
                field = process_field(field)
                n_fields++
                if (n_fields > 1) {
                    out_rec = out_rec "\t"
                }
                out_rec = out_rec field
            }
        }
        
        print out_rec
    }
    
    function process_field(f,    i, c, out, len, in_q, temp) {
        # Remove surrounding quotes if present
        len = length(f)
        if (len >= 2 && substr(f, 1, 1) == "\"" && substr(f, len, 1) == "\"") {
            f = substr(f, 2, len - 2)
            # Now we need to handle escaped quotes "" -> "
            # And escape tabs and newlines.
            # But wait, the field might have been split by newlines in the input record?
            # No, the `f` string contains the content, including newlines if the field spanned lines.
            # But the `process_record` function builds `f` by concatenating chars.
            # So `f` contains literal newlines and tabs.
            
            # Replace "" with "
            gsub(/""/, "\"", f)
            
            # Escape tabs and newlines
            gsub(/\t/, "\\t", f)
            gsub(/\n/, "\\n", f)
            
            return f
        } else {
            # Not quoted field
            # Just escape tabs and newlines
            gsub(/\t/, "\\t", f)
            gsub(/\n/, "\\n", f)
            return f
        }
    }
    
    END {
        # If there is a partial record left (in_quote is true), process it?
        # The problem statement says "The last record may have no trailing newline".
        # If the file ends with an incomplete record (unbalanced quotes), it's invalid.
        # We assume valid input.
        if (full_record != "") {
            process_record(full_record)
        }
    }
    ' "$file"
}