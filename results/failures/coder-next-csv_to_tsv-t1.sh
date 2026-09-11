csv_to_tsv() {
    local file="$1"
    local in_quotes=0
    local field=""
    local output=""
    local c
    local prev_c=""
    local -a record=()
    
    # Read file byte by byte to handle CRLF/LF and embedded newlines
    while IFS= read -r -d '' -n1 c || [[ -n "$c" ]] || [[ $in_quotes -eq 1 ]]; do
        # Remove carriage return if present
        if [[ "$c" == $'\r' ]]; then
            continue
        fi
        
        if [[ $in_quotes -eq 1 ]]; then
            if [[ "$c" == '"' ]]; then
                # Check for escaped quote ""
                if [[ "${prev_c}" == '"' ]]; then
                    field+='"'
                else
                    in_quotes=0
                fi
            else
                if [[ "$c" == $'\n' ]]; then
                    field+=$'\\n'
                elif [[ "$c" == $'\t' ]]; then
                    field+=$'\\t'
                else
                    field+="$c"
                fi
            fi
        else
            if [[ "$c" == '"' ]]; then
                # Only start quoted field at beginning or after comma
                if [[ -z "$field" ]] || [[ "${record[-1]}" == *","* ]]; then
                    in_quotes=1
                else
                    field+="$c"
                fi
            elif [[ "$c" == ',' ]]; then
                # End of field - unescape special sequences for output
                record+=("$field")
                field=""
            elif [[ "$c" == $'\n' ]]; then
                # End of record
                record+=("$field")
                field=""
                
                # Convert record to TSV line
                local tsv_line=""
                for i in "${!record[@]}"; do
                    if [[ $i -gt 0 ]]; then
                        tsv_line+=$'\t'
                    fi
                    tsv_line+="${record[i]}"
                done
                printf '%s\n' "$tsv_line"
                unset record
                record=()
            else
                field+="$c"
            fi
        fi
        
        prev_c="$c"
    done < "$file"
    
    # Handle last record if no trailing newline
    if [[ -n "$field" ]] || [[ ${#record[@]} -gt 0 ]]; then
        record+=("$field")
        local tsv_line=""
        for i in "${!record[@]}"; do
            if [[ $i -gt 0 ]]; then
                tsv_line+=$'\t'
            fi
            tsv_line+="${record[i]}"
        done
        printf '%s\n' "$tsv_line"
    fi
}