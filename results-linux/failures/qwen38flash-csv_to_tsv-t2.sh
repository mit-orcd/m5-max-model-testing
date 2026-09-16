csv_to_tsv() {
    local file="$1"
    local line field result line_buf
    local i len char next_char
    local in_quote=0
    local field_buf=""
    local record_buf=""
    local output_started=0

    # Read file byte by byte or line by line? 
    # Reading line by line is tricky with embedded newlines in quoted fields.
    # We need to parse the CSV structure.
    
    # Let's read the entire file into a variable or process it character by character.
    # Using a state machine.

    # Reset state
    in_quote=0
    field_buf=""
    record_buf=""

    # Read file content preserving newlines
    # We'll process character by character.
    
    local content
    # Read entire file
    content=$(cat "$file")
    # cat strips trailing newlines. We need to handle that.
    # Better approach: read line by line but handle multi-line records.

    # Alternative: Use awk or sed? No, complex logic.
    # Let's use a loop reading characters.
    
    # Re-reading the file to ensure we don't lose the last newline if it exists
    # Actually, let's use a different approach.
    # Read lines, accumulate until quote state is balanced.

    local line
    local pending_field=""
    local pending_record=""
    local pending_record_started=0
    
    # Reset
    in_quote=0
    pending_field=""
    pending_record=""

    # We need to handle the case where the last line doesn't end with newline.
    # `while IFS= read -r line` will miss the last line if no newline.
    # Use `|| [[ -n "$line" ]]` to catch it.

    while IFS= read -r line || [[ -n "$line" ]]; do
        # Strip CR from end of line (CRLF handling)
        line="${line%$'\r'}"
        
        local j=0
        local llen=${#line}
        
        while (( j < llen )); do
            char="${line:$j:1}"
            
            if (( in_quote )); then
                if [[ "$char" == '"' ]]; then
                    # Check for escaped quote ""
                    if (( j + 1 < llen )) && [[ "${line:$j+1:1}" == '"' ]]; then
                        pending_field+='"'
                        j=$((j + 2))
                        continue
                    else
                        # End of quoted field
                        in_quote=0
                        j=$((j + 1))
                        continue
                    fi
                else
                    # Inside quoted field: handle tabs and newlines (but newlines aren't here, they are separate lines)
                    # Tabs in quoted fields must be escaped as \t
                    if [[ "$char" == $'\t' ]]; then
                        pending_field+='\t'
                    else
                        pending_field+="$char"
                    fi
                    j=$((j + 1))
                fi
            else
                # Outside quoted field
                if [[ "$char" == '"' ]]; then
                    in_quote=1
                    j=$((j + 1))
                elif [[ "$char" == ',' ]]; then
                    # End of field
                    if [[ -z "$pending_record" ]]; then
                        pending_record="$pending_field"
                    else
                        pending_record+=$'\t'"$pending_field"
                    fi
                    pending_field=""
                    j=$((j + 1))
                else
                    # Normal character
                    # Tabs outside quotes? RFC 4180 doesn't explicitly forbid them in unquoted fields, 
                    # but usually unquoted fields shouldn't contain tabs. 
                    # However, if they do, they are literal tabs in CSV.
                    # In TSV, tabs are delimiters. So a literal tab in an unquoted CSV field 
                    # would break the structure. 
                    # But the prompt says "a tab inside a field as \t". This applies to any field.
                    if [[ "$char" == $'\t' ]]; then
                        pending_field+='\t'
                    else
                        pending_field+="$char"
                    fi
                    j=$((j + 1))
                fi
            fi
        done
        
        # End of line reached.
        if (( in_quote )); then
            # We are inside a quoted field. The newline is part of the field.
            # Append \n to pending_field (representing the newline in the quoted field)
            # Then continue to next line.
            pending_field+='\n'
        else
            # End of record.
            if [[ -z "$pending_record" ]]; then
                pending_record="$pending_field"
            else
                pending_record+=$'\t'"$pending_field"
            fi
            
            # Output the record
            printf '%s\n' "$pending_record"
            
            # Reset for next record
            pending_field=""
            pending_record=""
            # in_quote is already 0
        fi
    done < "$file"

    # Handle case where file ends inside a quoted field (malformed CSV, but let's be safe)
    # Or if there's pending data not output.
    # If in_quote is 1 after loop, it means unclosed quote. 
    # The prompt implies valid CSV.
    # But if pending_record has content (e.g. last line without newline, but quote closed), 
    # the loop above outputs it when in_quote becomes 0 at end of line.
    
    # Wait, the loop logic above outputs when `in_quote` is 0 at end of line.
    # If the file ends without a newline, `read` returns non-zero, but `line` is set.
    # The loop runs. If it ends with quote closed, it outputs.
    # If it ends with quote open, it doesn't output.
    
    # If we have pending field/record due to unclosed quote at EOF?
    if [[ -n "$pending_field" || -n "$pending_record" ]]; then
        if [[ -z "$pending_record" ]]; then
             pending_record="$pending_field"
        else
             pending_record+=$'\t'"$pending_field"
        fi
        printf '%s\n' "$pending_record"
    fi
}