csv_to_tsv() {
    local file="$1"
    local IFS=$'\n'
    local line field in_quote escaped prev_char result lines=() i
    while IFS= read -r line || [[ -n $line ]]; do
        # Remove trailing CR if present
        line="${line%$'\r'}"
        field=""
        in_quote=0
        escaped=0
        prev_char=""
        result=""
        for (( i=0; i<${#line}; i++ )); do
            c="${line:i:1}"
            if [[ $escaped -eq 1 ]]; then
                if [[ $c == '"' ]]; then
                    result+='"'
                else
                    # Should not happen per RFC-4180, but treat as literal
                    result+='"'$c
                fi
                escaped=0
            elif [[ $in_quote -eq 1 ]]; then
                if [[ $c == '"' ]]; then
                    if [[ $i -lt $((${#line}-1)) && "${line:i+1:1}" == '"' ]]; then
                        # Escaped quote
                        result+='"'
                        ((i++)) # Skip next quote
                    else
                        # End of quoted field
                        in_quote=0
                    fi
                else
                    result+="$c"
                fi
            else
                if [[ $c == ',' ]]; then
                    # End of field
                    # Replace literal tab and newline in field with \t and \n
                    field="${result}"
                    field="${field//$'\t'/\\t}"
                    field="${field//$'\n'/\\n}"
                    printf '%s\t' "$field"
                    result=""
                elif [[ $c == '"' ]]; then
                    in_quote=1
                else
                    result+="$c"
                fi
            fi
        done
        # Last field
        field="${result}"
        field="${field//$'\t'/\\t}"
        field="${field//$'\n'/\\n}"
        printf '%s\n' "$field"
    done < "$file"
}