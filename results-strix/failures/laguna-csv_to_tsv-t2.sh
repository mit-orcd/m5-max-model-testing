csv_to_tsv() {
    local file="$1"
    local in_quotes=0
    local field=""
    local line=""
    local char prev_char=""
    
    # Read file character by character, handling CRLF and LF
    while IFS= read -r -n1 char || [[ -n "$char" ]]; do
        # Handle CRLF as LF
        if [[ "$prev_char" == $'\r' && "$char" == $'\n' ]]; then
            continue
        fi
        
        if [[ $in_quotes -eq 1 ]]; then
            if [[ "$char" == '"' ]]; then
                if [[ "$prev_char" == '"' ]]; then
                    # Escaped quote, add single quote
                    field+='"'
                    char=""
                else
                    # End of quoted field
                    in_quotes=0
                fi
            else
                field+="$char"
            fi
        else
            if [[ "$char" == '"' ]]; then
                in_quotes=1
            elif [[ "$char" == $'\t' ]]; then
                # Unquoted tab - should not happen in valid CSV, but handle
                line+=$'\t'
            elif [[ "$char" == $'\n' ]]; then
                # End of record
                # Escape tabs and line breaks in field
                field="${field//$'\t'/\\t}"
                field="${field//$'\n'/\\n}"
                line+="${field}"$'\n'
                echo -e "$line"
                line=""
                field=""
                continue
            elif [[ "$char" == ',' ]]; then
                # End of field, add tab
                field="${field//$'\t'/\\t}"
                field="${field//$'\n'/\\n}"
                line+="${field}"$'\t'
                field=""
                continue
            else
                field+="$char"
            fi
        fi
        prev_char="$char"
    done < "$file"
    
    # Handle last record without trailing newline
    if [[ -n "$field" || -n "$line" ]]; then
        field="${field//$'\t'/\\t}"
        field="${field//$'\n'/\\n}"
        line+="${field}"
        echo -e "$line"
    fi
}