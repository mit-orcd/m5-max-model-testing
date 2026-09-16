csv_to_tsv() {
    local file="$1"
    local in_quote=0
    local field=""
    local output=""
    local c

    while IFS= read -r -n 1 c || [[ -n $c ]]; do
        if [[ $in_quote -eq 1 ]]; then
            if [[ $c == '"' ]]; then
                # Peek next char to check for escaped quote
                local next
                read -r -n 1 next || true
                if [[ $next == '"' ]]; then
                    field+='"'
                    # Consume the extra quote
                    read -r -n 1 || true
                else
                    in_quote=0
                fi
            else
                field+="$c"
            fi
        else
            if [[ $c == '"' ]]; then
                in_quote=1
            elif [[ $c == $'\t' ]]; then
                output+="$field"$'\t'
                field=""
            elif [[ $c == $'\n' ]]; then
                output+="$field"$'\n'
                field=""
            elif [[ $c == $'\r' ]]; then
                # Ignore CR, wait for LF
                :
            else
                field+="$c"
            fi
        fi
    done < "$file"

    # Handle last field if file didn't end with newline
    if [[ -n $field ]]; then
        output+="$field"$'\n'
    fi

    printf '%s' "$output"
}