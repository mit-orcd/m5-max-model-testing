csv_to_tsv() {
    local file="$1"
    local line field in_quotes=0 i c prev_c escaped=0
    local IFS=$'\n'
    while IFS= read -r line || [[ -n $line ]]; do
        line="${line%$'\r'}"
        field=""
        in_quotes=0
        escaped=0
        for (( i=0; i<${#line}; i++ )); do
            c="${line:i:1}"
            if [[ $escaped -eq 1 ]]; then
                if [[ "$c" == '"' ]]; then
                    field+='"'
                else
                    field+='"'"$c"
                fi
                escaped=0
                continue
            fi
            if [[ "$c" == '"' ]]; then
                if [[ $in_quotes -eq 1 ]]; then
                    if [[ $((i+1)) -lt ${#line} && "${line:i+1:1}" == '"' ]]; then
                        field+='"'
                        ((i++))
                    else
                        in_quotes=0
                    fi
                else
                    in_quotes=1
                fi
            elif [[ "$c" == ',' && $in_quotes -eq 0 ]]; then
                if [[ $field == *$'\n'* || $field == *$'\t'* ]]; then
                    field="${field//$'\n'/\\n}"
                    field="${field//$'\t'/\\t}"
                fi
                printf '%s\t' "$field"
                field=""
            else
                field+="$c"
            fi
        done
        if [[ $field == *$'\n'* || $field == *$'\t'* ]]; then
            field="${field//$'\n'/\\n}"
            field="${field//$'\t'/\\t}"
        fi
        printf '%s\n' "$field"
    done < "$file"
}