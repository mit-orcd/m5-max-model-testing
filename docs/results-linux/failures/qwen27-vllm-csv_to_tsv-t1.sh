csv_to_tsv() {
    local file="$1"
    local line
    local field
    local in_quotes=0
    local current_field=""
    local current_record=()
    local c
    local prev_c=""
    local i

    # Read file content
    local content
    content=$(cat -- "$file")
    # Preserve trailing newlines issue - use a different approach
    # Read character by character using a while loop with file descriptor

    exec 3< "$file"
    
    local record=""
    local field=""
    local in_quotes=0
    local eof=0
    
    while IFS= read -r -n1 c <&3 || { [[ -n "$c" ]] && eof=1; }; do
        if [[ $eof -eq 1 && -z "$c" ]]; then
            break
        fi
        
        if [[ $in_quotes -eq 1 ]]; then
            if [[ "$c" == '"' ]]; then
                # Check if next char is also a quote
                local next_c=""
                IFS= read -r -n1 next_c <&3 || next_c=""
                if [[ "$next_c" == '"' ]]; then
                    # Escaped quote
                    field+='"'
                else
                    # End of quoted field
                    in_quotes=0
                fi
            elif [[ "$c" == $'\n' ]]; then
                field+='\n'
            elif [[ "$c" == $'\t' ]]; then
                field+='\t'
            elif [[ "$c" == $'\r' ]]; then
                # CR inside quotes - ignore per RFC (or treat as part of field?)
                # RFC 4180 says CRLF is the line ending, but inside quotes it's part of data
                # However, the spec says "a line break inside a quoted field is written as \n"
                # So CR should be stripped if followed by LF, or treated as line break?
                # Let's treat CR as part of the line break sequence
                # If next is LF, skip both and add \n
                # If standalone, treat as \n? 
                # Actually, let's just skip CR when in quotes since we handle LF
                # But what if it's a lone CR? Let's add \n for CR too if not followed by LF
                # For simplicity, skip CR in quotes (it's part of CRLF which we handle via LF)
                :
            else
                field+="$c"
            fi
        else
            if [[ "$c" == '"' ]]; then
                in_quotes=1
            elif [[ "$c" == ',' ]]; then
                current_record+=("$field")
                field=""
            elif [[ "$c" == $'\n' ]]; then
                current_record+=("$field")
                field=""
                # Output the record
                local out=""
                local j
                for ((j=0; j<${#current_record[@]}; j++)); do
                    if [[ $j -gt 0 ]]; then
                        out+=$'\t'
                    fi
                    out+="${current_record[$j]}"
                done
                printf '%s\n' "$out"
                current_record=()
            elif [[ "$c" == $'\r' ]]; then
                # CR - check if next is LF
                local next_c=""
                IFS= read -r -n1 next_c <&3 || next_c=""
                if [[ "$next_c" == $'\n' ]]; then
                    # CRLF - end of record
                    current_record+=("$field")
                    field=""
                    local out=""
                    local j
                    for ((j=0; j<${#current_record[@]}; j++)); do
                        if [[ $j -gt 0 ]]; then
                            out+=$'\t'
                        fi
                        out+="${current_record[$j]}"
                    done
                    printf '%s\n' "$out"
                    current_record=()
                else
                    # Lone CR - treat as line ending? Or ignore?
                    # Per spec, CR must not survive. Treat as line break.
                    current_record+=("$field")
                    field=""
                    local out=""
                    local j
                    for ((j=0; j<${#current_record[@]}; j++)); do
                        if [[ $j -gt 0 ]]; then
                            out+=$'\t'
                        fi
                        out+="${current_record[$j]}"
                    done
                    printf '%s\n' "$out"
                    current_record=()
                fi
            else
                field+="$c"
            fi
        fi
    done
    
    # Handle last record if no trailing newline
    if [[ -n "$field" || ${#current_record[@]} -gt 0 ]]; then
        current_record+=("$field")
        local out=""
        local j
        for ((j=0; j<${#current_record[@]}; j++)); do
            if [[ $j -gt 0 ]]; then
                out+=$'\t'
            fi
            out+="${current_record[$j]}"
        done
        printf '%s\n' "$out"
    fi
    
    exec 3<&-
}