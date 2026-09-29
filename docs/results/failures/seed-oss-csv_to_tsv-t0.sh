csv_to_tsv() {
    local -a fields
    local field="" quoted=0 crlf=0
    local -i i=0 c
    while IFS= read -r -n1 c; do
        if [[ $c == $'\r' ]]; then
            crlf=1
            continue
        elif [[ $c == $'\n' ]]; then
            if (( quoted )); then
                field+='\\n'
            else
                if (( i > 0 )); then
                    printf '%s\t' "${fields[@]}"
                else
                    printf '%s\n' "$field"
                fi
                fields=()
                field=""
                quoted=0
                i=0
                (( crlf )) && crlf=0 || printf '\n'
            fi
        elif [[ $c == ',' && ! $quoted ]]; then
            fields+=("$field")
            field=""
            ((i++))
        elif [[ $c == '"' ]]; then
            if [[ $field == *$'"'$'"' ]]; then
                field="${field%??}"
            else
                quoted=$((quoted ^ 1))
            fi
        elif [[ $c == $'\t' && $quoted ]]; then
            field+='\\t'
        else
            field+="$c"
        fi
    done < "$1"
    if [[ -n $field || $quoted ]]; then
        if (( i > 0 )); then
            printf '%s\t' "${fields[@]}"
        fi
        printf '%s\n' "$field"
    fi
}