csv_to_tsv() {
    local file="$1"
    local line
    while IFS= read -r line || [[ -n "$line" ]]; do
        local record=()
        local field=""
        local in_quotes=0
        for (( i=0; i<${#line}; i++ )); do
            char="${line:$i:1}"
            if [[ "$in_quotes" -eq 1 ]]; then
                if [[ "$char" == "\"" ]]; then
                    if [[ $((i+1)) -lt ${#line} && "${line:$((i+1)):1}" == "\"" ]]; then
                        field+="\""
                        ((i++))
                    else
                        in_quotes=0
                    fi
                elif [[ "$char" == $'\n' ]]; then
                    field+=$'\\n'
                elif [[ "$char" == $'\t' ]]; then
                    field+=$'\\t'
                else
                    field+="$char"
                fi
            else
                if [[ "$char" == "\"" ]]; then
                    in_quotes=1
                elif [[ "$char" == "," ]]; then
                    record+=("$field")
                    field=""
                else
                    field+="$char"
                fi
            fi
        done
        record+=("$field")
        printf "%s\n" "$(IFS=$'\t'; echo "${record[*]}")"
    done < "$file"
}