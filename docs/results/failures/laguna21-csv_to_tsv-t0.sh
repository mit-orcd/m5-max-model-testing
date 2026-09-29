csv_to_tsv() {
    local file="$1"
    local in_quotes=0
    local field=""
    local line=""
    local char=""
    local prev_char=""
    local i=0
    local len=0
    
    while IFS= read -r line || [[ -n "$line" ]]; do
        # Normalize line endings: remove carriage return if present
        line="${line%$'\r'}"
        
        len=${#line}
        i=0
        
        while (( i < len )); do
            char="${line:$i:1}"
            
            if [[ "$char" == '"' ]]; then
                if (( in_quotes )); then
                    if [[ "$prev_char" == '"' ]]; then
                        # Escaped quote: add single quote
                        field+='"'
                        in_quotes=0
                    else
                        # Closing quote
                        in_quotes=0
                    fi
                else
                    # Opening quote
                    in_quotes=1
                fi
            else
                if (( in_quotes )); then
                    case "$char" in
                        $'\t') field+='\t' ;;
                        $'\n') field+='\n' ;;
                        $'\r') field+='\r' ;;
                        *) field+="$char" ;;
                    esac
                else
                    field+="$char"
                fi
            fi
            
            prev_char="$char"
            (( i++ ))
        done
        
        # Output the field (replace special chars with their escapes)
        echo -n "$field" | sed 's/\t/\\t/g; s/\n/\\n/g; s/\r//g'
        
        # Output newline for this record
        echo
        
        # Reset for next record
        field=""
        line=""
    done < "$file"
}