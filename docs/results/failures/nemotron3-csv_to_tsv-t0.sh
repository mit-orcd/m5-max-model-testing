csv_to_tsv() {
    local file="$1"
    local line field in_quotes=0 i c prev_c escaped=0
    local -a fields
    local IFS=$'\n'
    while IFS= read -r line || [[ -n $line ]]; do
        line="${line%$'\r'}"
        fields=()
        field=""
        in_quotes=0
        escaped=0
        for (( i=0; i<${#line}; i++ )); do
            c="${line:i:1}"
            if [[ $escaped -eq 1 ]]; then
                if [[ $c == '"' ]]; then
                    field+='"'
                else
                    field+='""'"$c"
                fi
                escaped=0
                continue
            fi
            if [[ $in_quotes -eq 1 ]]; then
                if [[ $c == '"' ]]; then
                    if [[ $i -lt $((${#line}-1)) && "${line:i+1:1}" == '"' ]]; then
                        field+='"'
                        ((i++))
                    else
                        in_quotes=0
                    fi
                else
                    if [[ $c == $'\t' ]]; then
                        field+=$'\\t'
                    elif [[ $c == $'\n' ]]; then
                        field+=$'\\n'
                    else
                        field+="$c"
                    fi
                fi
            else
                if [[ $c == '"' ]]; then
                    in_quotes=1
                elif [[ $c == ',' ]]; then
                    fields+=("$field")
                    field=""
                else
                    field+="$c"
                fi
            fi
        done
        if [[ $in_quotes -eq 1 ]]; then
            field+='"'
        fi
        fields+=("$field")
        printf '%s\n' "${fields[*]}" | tr ' ' '\t'
    done < "$file"
    printf '\n'
}