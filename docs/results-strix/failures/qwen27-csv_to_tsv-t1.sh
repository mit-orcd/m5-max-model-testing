csv_to_tsv() {
    local file="$1"
    local line field record
    local -a fields
    local in_quotes=0
    local prev_char=""
    local cur_field=""
    local record=""

    # Read the file, handling CRLF and LF line endings
    # We process character by character to handle quoted fields spanning multiple lines

    # Read all content, stripping CR
    local content
    content=$(<"$file")

    # We need to handle the case where the file may not end with a newline
    # $(<file) strips trailing newlines, so we need to be careful

    # Let's use a different approach: read the file byte by byte using a while loop
    # But bash doesn't easily do byte-by-byte. Let's use a different strategy.

    # Actually, let's re-read: we need to parse CSV properly.
    # Let's use a state machine approach reading the file.

    # Reset
    in_quotes=0
    cur_field=""
    record=""

    # Read file character by character
    # We'll use a trick: read the file into a variable preserving everything,
    # then iterate.

    # Better approach: use while read with IFS= and -r to preserve lines,
    # but we need to handle quoted fields that span lines.
    
    # Let's do a proper character-by-character parse.
    # Read the file content preserving all characters except we handle CR.

    # Use a temporary approach: read the entire file
    local data
    data=$(cat "$file")

    # The data variable has trailing newlines stripped by command substitution.
    # We need to track if the original file ended with a newline.
    # Actually, for CSV parsing, the last record may or may not have a trailing newline.
    # The key is to parse records correctly.

    # Let me use a different, more robust approach with a byte-by-byte reader.
    
    # Since we can't easily do byte-by-byte in bash, let's use a different strategy:
    # Process the file line by line, but maintain state across lines for quoted fields.

    local -a records=()
    local cur_record=""
    local in_field_quote=0
    local cur_field=""
    local -a fields=()
    local eof=0

    # We'll read the file line by line
    while IFS= read -r line || [[ -n "$line" ]]; do
        # Remove trailing CR if present
        line="${line%$'\r'}"

        local i=0
        local len=${#line}
        while (( i < len )); do
            local ch="${line:i:1}"
            
            if (( in_field_quote )); then
                if [[ "$ch" == '"' ]]; then
                    # Check if next char is also a quote (escaped quote)
                    if (( i + 1 < len )) && [[ "${line:i+1:1}" == '"' ]]; then
                        cur_field+="\""
                        ((i++))
                    else
                        in_field_quote=0
                    fi
                else
                    cur_field+="$ch"
                fi
            else
                if [[ "$ch" == '"' ]]; then
                    in_field_quote=1
                elif [[ "$ch" == ',' ]]; then
                    fields+=("$cur_field")
                    cur_field=""
                elif [[ "$ch" == $'\n' ]]; then
                    # This shouldn't happen since we read line by line
                    :
                else
                    cur_field+="$ch"
                fi
            fi
            ((i++))
        done

        # After processing the line, check if we're still in a quoted field
        if (( in_field_quote )); then
            # The quoted field continues to the next line
            # Add a newline to the current field (will be converted to \n later)
            cur_field+=$'\n'
        else
            # Field is complete
            fields+=("$cur_field")
            cur_field=""
            # Record is complete
            records+=("${fields[*]}$'\t'") # This is wrong, let me fix
            # Actually, let's join fields with tab
            local joined=""
            local -a farr=("${fields[@]}")
            local j
            for (( j=0; j<${#farr[@]}; j++ )); do
                if (( j > 0 )); then
                    joined+=$'\t'
                fi
                joined+="${farr[j]}"
            done
            records+=("$joined")
            fields=()
        fi
    done < "$file"

    # If there are remaining fields (last record without trailing newline)
    if (( ${#fields[@]} > 0 )) || [[ -n "$cur_field" ]]; then
        fields+=("$cur_field")
        local joined=""
        local -a farr=("${fields[@]}")
        local j
        for (( j=0; j<${#farr[@]}; j++ )); do
            if (( j > 0 )); then
                joined+=$'\t'
            fi
            joined+="${farr[j]}"
        done
        records+=("$joined")
    fi

    # Now process each record: replace actual newlines with \n and actual tabs with \t
    local r
    for r in "${records[@]}"; do
        # Replace newline with \n
        r="${r//$'\n'/\\n}"
        # Replace tab with \t
        r="${r//$'\t'/\\t}"
        printf '%s\n' "$r"
    done
}