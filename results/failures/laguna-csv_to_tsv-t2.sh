csv_to_tsv() {
    local file="$1"
    local field=""
    local in_quotes=false
    local output_line=""
    local first_field=true
    local char=""
    local next_char=""
    local escaped_char=""
    
    while IFS= read -r -n1 char < "$file" || [[ -n "$char" ]]; do
        if $in_quotes; then
            if [[ "$char" == '"' ]]; then
                read -n1 next_char < "$file" 2>/dev/null || next_char=""
                if [[ "$next_char" == '"' ]]; then
                    field+='""'
                    continue
                else
                    in_quotes=false
                    if [[ -n "$next_char" ]]; then
                        char="$next_char"
                    else
                        char=""
                    fi
                fi
            elif [[ "$char" == $'\n' ]]; then
                field+='\\n'
                continue
            elif [[ "$char" == $'\t' ]]; then
                field+='\\t'
                continue
            else
                field+="$char"
                continue
            fi
        fi
        
        if [[ "$char" == '"' ]]; then
            in_quotes=true
            field=""
        elif [[ "$char" == ',' ]]; then
            if $first_field; then
                output_line="$field"
                first_field=false
            else
                output_line="$output_line"$'\t'"$field"
            fi
            field=""
        elif [[ "$char" == $'\r' ]]; then
            continue
        elif [[ "$char" == $'\n' ]]; then
            if $first_field; then
                output_line="$field"
            else
                output_line="$output_line"$'\t'"$field"
            fi
            echo "$output_line"
            output_line=""
            field=""
            first_field=true
        else
            field+="$char"
        fi
    done < "$file"
    
    if $first_field; then
        output_line="$field"
    else
        output_line="$output_line"$'\t'"$field"
    fi
    echo "$output_line"
}