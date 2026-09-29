csv_to_tsv() {
    local file="$1"
    local in_quotes=0
    local field=""
    local line=""
    local char=""
    local prev_char=""
    
    while IFS= read -r line || [[ -n "$line" ]]; do
        # Normalize line endings: remove carriage returns
        line="${line%$'\r'}"
        
        prev_char=""
        
        for (( i=0; i<${#line}; i++ )); do
            char="${line:$i:1}"
            
            if [[ "$char" == '"' ]]; then
                if [[ "$prev_char" == '"' ]]; then
                    # Escaped quote, add single quote to field
                    field+='"'
                    prev_char=""
                else
                    # Toggle quote state
                    in_quotes=$((1 - in_quotes))
                    prev_char=""
                fi
            elif [[ "$char" == $'\t' && "$in_quotes" -eq 0 ]]; then
                # Unquoted tab: field separator
                printf '%s\t' "$field"
                field=""
            elif [[ "$char" == $'\n' && "$in_quotes" -eq 0 ]]; then
                # Unquoted newline: record separator
                printf '%s\n' "$field"
                field=""
            elif [[ "$char" == $'\t' && "$in_quotes" -eq 1 ]]; then
                # Tab inside quoted field
                field+='\\t'
            elif [[ "$char" == $'\n' && "$in_quotes" -eq 1 ]]; then
                # Newline inside quoted field
                field+='\\n'
            else
                field+="$char"
            fi
            
            prev_char="$char"
        done
        
        # Output last field of the record
        printf '%s\n' "$field"
        field=""
    done < "$file"
}