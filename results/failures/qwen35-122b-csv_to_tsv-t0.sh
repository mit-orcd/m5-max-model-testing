csv_to_tsv() {
    local file="$1"
    local IFS=
    local line=""
    local field=""
    local in_quotes=0
    local char
    local output=""
    local first_field=1

    # Read file character by character
    while IFS= read -r -d '' -n 1 char || [[ -n "$char" ]]; do
        # Handle CRLF -> LF
        if [[ "$char" == $'\r' ]]; then
            continue
        fi

        if [[ $in_quotes -eq 1 ]]; then
            if [[ "$char" == '"' ]]; then
                # Check for escaped quote ""
                # We need to peek ahead, but read -n 1 consumes.
                # Strategy: read next char to check
                local next_char
                if IFS= read -r -d '' -n 1 next_char 2>/dev/null; then
                    if [[ "$next_char" == '"' ]]; then
                        # Escaped quote, add one quote and continue
                        field+='"'
                    else
                        # End of quoted field
                        in_quotes=0
                        # Put back the next_char? No, we can't easily put back.
                        # Instead, we handle the logic: if next was not quote, it's a delimiter or end.
                        # But we consumed it. We need to process it in the next iteration.
                        # This approach is flawed for state machines in bash without complex buffering.
                        # Let's switch to a line-based approach with careful parsing.
                        :
                    fi
                else
                    # End of file after quote
                    in_quotes=0
                fi
            else
                field+="$char"
            fi
        else
            if [[ "$char" == '"' ]]; then
                in_quotes=1
            elif [[ "$char" == $'\n' ]]; then
                # End of record
                # Escape tabs and newlines in field (though newlines inside quotes are handled by in_quotes)
                # But we need to handle tabs inside quoted fields.
                # Since we are processing char by char, we can escape on the fly.
                # However, the requirement says: "a line break inside a quoted field is written as \n"
                # and "tab inside a field as \t".
                # Since we are in the non-quoted state, we just output the field.
                # Wait, the logic above for in_quotes=1 needs to handle newlines and tabs.
                # Let's restart with a cleaner state machine that buffers the field.
                :
            elif [[ "$char" == $'\t' ]]; then
                # Tab outside quotes? RFC 4180 says tabs are allowed in unquoted fields?
                # Usually TSV implies tabs are delimiters. If a tab is in an unquoted field, it breaks TSV.
                # The prompt says: "Because a tab and a newline cannot appear literally in TSV..."
                # This implies we must escape them even if they appear in unquoted fields?
                # Or does it mean we only escape them if they were inside quotes?
                # "a tab inside a field as the two characters backslash t"
                # If a tab is in an unquoted field, it would split the TSV.
                # The prompt implies the input is RFC 4180 CSV. In CSV, tabs are allowed in unquoted fields.
                # But TSV cannot have literal tabs. So we must escape them.
                field+=$'\\t'
            else
                field+="$char"
            fi
        fi
    done < "$file"

    # This character-by-character approach in bash is extremely slow and hard to manage state for "peeking".
    # Let's use a more robust method: Read line by line, but handle multi-line quoted fields.
    # We will read the whole file into a variable or process it with a custom parser loop.
    # Given the constraints (no python/perl), we must use bash.
    
    # Re-implementation:
    # We will read the file into an array of characters? No, too memory heavy.
    # We will read line by line, but if we are in quotes, we append to the current record.
    
    local record=""
    local in_quotes=0
    local line
    local field=""
    local first_field=1
    local output_line=""
    
    # Function to escape a field for TSV
    escape_field() {
        local f="$1"
        # Replace \ with \\? No, prompt doesn't say escape backslash.
        # Replace " with ""? No, quotes are removed.
        # Replace tab with \t
        # Replace newline with \n (but newlines only happen inside quotes)
        # We need to handle the fact that we are building the field string.
        # Since we are building the field in bash, we can do the escaping as we go or at the end.
        # Escaping at the end is easier.
        # But bash string manipulation is slow.
        # Let's do it at the end using parameter expansion or sed?
        # sed is allowed.
        # We need to escape:
        # 1. Tabs -> \t
        # 2. Newlines -> \n (only if they were inside quotes, but we are reconstructing the field)
        # Wait, if we read line by line, we lose the newline character if it was inside quotes.
        # We need to preserve it as a literal newline in the variable, then escape it.
        
        # Replace literal tab with \t
        # Replace literal newline with \n
        # Note: Bash variables can contain newlines.
        f="${f//$'\t'/\\t}"
        f="${f//$'\n'/\\n}"
        printf '%s' "$f"
    }

    # Reset variables
    record=""
    in_quotes=0
    first_field=1
    output_line=""
    
    # Read file line by line, preserving newlines in quoted fields by concatenating
    while IFS= read -r line || [[ -n "$line" ]]; do
        # If we are in quotes, we append the line (plus a newline) to the current record
        if [[ $in_quotes -eq 1 ]]; then
            record+=$'\n'"$line"
        else
            record="$line"
        fi
        
        # Count quotes in the current line to determine if we are still in quotes
        # We need to count unescaped quotes.
        # This is tricky. A quote is escaped if it is followed by another quote.
        # Let's iterate through the line to update in_quotes state.
        local i=0
        local len=${#line}
        local char
        local temp_line="$line"
        
        # We need to process the line character by character to handle quote toggling correctly
        # especially for "" sequences.
        # But we already appended the line to record.
        # Let's re-evaluate: We need to know if the record is complete.
        # A record is complete if the number of unescaped quotes is even.
        
        # Let's count quotes in the current record? No, that's expensive.
        # Let's track state as we read.
        # We need to re-process the line to update in_quotes.
        
        # Reset in_quotes for the current line processing? No, it's a state.
        # We need to count how many quotes are in this line that toggle the state.
        # But we must handle "" as a single quote.
        
        # Let's use a temporary variable to count toggles.
        local toggle_count=0
        local j=0
        local l_len=${#line}
        while (( j < l_len )); do
            char="${line:j:1}"
            if [[ "$char" == '"' ]]; then
                # Check if next is also quote
                if (( j + 1 < l_len )) && [[ "${line:j+1:1}" == '"' ]]; then
                    # Escaped quote, skip next char
                    (( j += 2 ))
                    continue
                else
                    # Toggle state
                    (( toggle_count++ ))
                fi
            fi
            (( j++ ))
        done
        
        if (( toggle_count % 2 == 1 )); then
            in_quotes=1
        else
            in_quotes=0
            # Record is complete
            # Now parse the record into fields
            # We need to split the record by commas, respecting quotes.
            # This is complex to do in pure bash without external tools.
            # But we can use a loop.
            
            local current_field=""
            local in_field_quotes=0
            local k=0
            local r_len=${#record}
            local field_list=()
            
            while (( k < r_len )); do
                char="${record:k:1}"
                if [[ $in_field_quotes -eq 1 ]]; then
                    if [[ "$char" == '"' ]]; then
                        # Check for ""
                        if (( k + 1 < r_len )) && [[ "${record:k+1:1}" == '"' ]]; then
                            current_field+='"'
                            (( k += 2 ))
                            continue
                        else
                            in_field_quotes=0
                        fi
                    else
                        current_field+="$char"
                    fi
                else
                    if [[ "$char" == '"' ]]; then
                        in_field_quotes=1
                    elif [[ "$char" == ',' ]]; then
                        field_list+=("$current_field")
                        current_field=""
                    else
                        current_field+="$char"
                    fi
                fi
                (( k++ ))
            done
            field_list+=("$current_field")
            
            # Output the TSV line
            local tsv_line=""
            local first=1
            for fld in "${field_list[@]}"; do
                # Escape tabs and newlines in the field
                fld="${fld//$'\t'/\\t}"
                fld="${fld//$'\n'/\\n}"
                if [[ $first -eq 1 ]]; then
                    tsv_line="$fld"
                    first=0
                else
                    tsv_line+=$'\t'"$fld"
                fi
            done
            printf '%s\n' "$tsv_line"
            
            record=""
        fi
    done < "$file"
    
    # Handle last record if file didn't end with newline and we were in quotes?
    # The loop condition `|| [[ -n "$line" ]]` handles the last line without newline.
    # If we are still in_quotes after the loop, we need to process the remaining record.
    if [[ $in_quotes -eq 1 ]] && [[ -n "$record" ]]; then
        local current_field=""
        local in_field_quotes=0
        local k=0
        local r_len=${#record}
        local field_list=()
        
        while (( k < r_len )); do
            char="${record:k:1}"
            if [[ $in_field_quotes -eq 1 ]]; then
                if [[ "$char" == '"' ]]; then
                    if (( k + 1 < r_len )) && [[ "${record:k+1:1}" == '"' ]]; then
                        current_field+='"'
                        (( k += 2 ))
                        continue
                    else
                        in_field_quotes=0
                    fi
                else
                    current_field+="$char"
                fi
            else
                if [[ "$char" == '"' ]]; then
                    in_field_quotes=1
                elif [[ "$char" == ',' ]]; then
                    field_list+=("$current_field")
                    current_field=""
                else
                    current_field+="$char"
                fi
            fi
            (( k++ ))
        done
        field_list+=("$current_field")
        
        local tsv_line=""
        local first=1
        for fld in "${field_list[@]}"; do
            fld="${fld//$'\t'/\\t}"
            fld="${fld//$'\n'/\\n}"
            if [[ $first -eq 1 ]]; then
                tsv_line="$fld"
                first=0
            else
                tsv_line+=$'\t'"$fld"
            fi
        done
        printf '%s\n' "$tsv_line"
    fi
}