csv_to_tsv() {
    local IFS=$'\n'
    local -a lines=()
    local line buffer quote state=off

    while IFS= read -r -d '' line; do
        lines+=("$line")
    done < <(tr -d '\r' < "$1")

    for line in "${lines[@]}"; do
        if [[ -z "$line" ]]; then
            echo -n -e '\n'
            continue
        fi

        buffer=""
        quote=off        
        local i=0
        local len=${#line        
        while (( i < len )); do
            local char="${line:i:1}"
            
            if [[ "$quote" == off ]]; then
                if [[ "$char" == '"' ]]; then
                    quote=on
                else
                    buffer+="$char"
                fi
            else
                if [[ "$char" == '"' ]]; then
                    if (( i + 1 < len )) && [[ "${line:i+1:1}" == '"' ]]; then
                        buffer+='"'
                        ((i++))
                    else
                        quote=off
                    fi
                else
                    buffer+="$char"
                fi
            fi
            ((i++))
        done

        if [[ -n "$buffer" ]]; then
            if [[ "$state" == off ]]; then
                echo -n -e "$buffer"
            else
                echo -e "$buffer"
            fi
        fi
        state=on
    done

    if [[ "${lines[*]}" != "" ]]; then
        echo -n -e '\n'
    fi
}