csv_to_tsv() {
    local file="$1"
    local -r input="$(cat "$file" | tr -d '\r')"
    local -r len=${#input}
    local i=0
    local field=""
    local in_quotes=0
    local -a fields=()
    
    while (( i < len )); do
        local char="${input:$i:1}"
        local next_char=""
        (( i + 1 < len )) && next_char="${input:$((i+1)):1}"
        
        if (( in_quotes )); then
            if [[ "$char" == '"' ]]; then
                if [[ "$next_char" == '"' ]]; then
                    field+=$'\n'
                    field="${field%$'\n'}\""
                    (( i++ ))
                else
                    in_quotes=0
                    (( i++ ))
                fi
            elif [[ "$char" == $'\n' ]]; then
                field+='\n'
            elif [[ "$char" == $'\t' ]]; then
                field+='\t'
            else
                field+="$char"
                (( i++ ))
            fi
        else
            if [[ "$char" == '"' ]]; then
                in_quotes=1
                (( i++ ))
            elif [[ "$char" == ',' ]]; then
                fields+=("$field")
                field=""
                (( i++ ))
            elif [[ "$char" == $'\n' ]]; then
                fields+=("$field")
                field=""
                (( i++ ))
                printf '%s\n' "${fields[*]}"
                fields=()
            else
                field+="$char"
                (( i++ ))
            fi
        fi
    done
    
    if [[ -n "$field" ]] || (( ${#fields[@]} > 0 )); then
        fields+=("$field")
        printf '%s\n' "${fields[*]}"
    fi
}