csv_to_tsv() {
    local file="$1"
    local line field fields=() in_quoted=0 i c next_char unescaped=""
    local -a records
    records=()

    while IFS= read -r line || [[ -n "$line" ]]; do
        # Normalize line endings (remove CR)
        line="${line//$'\r'/}"
        fields=()
        in_quoted=0
        field=""
        i=0
        len=${#line}
        while (( i < len )); do
            c="${line:i:1}"
            if (( in_quoted )); then
                if [[ "$c" == '"' ]]; then
                    next_char="${line:i+1:1}"
                    if [[ "$next_char" == '"' ]]; then
                        field+='"'
                        ((i+=2))
                    else
                        in_quoted=0
                        ((i++))
                    fi
                else
                    field+="$c"
                    ((i++))
                fi
            else
                if [[ "$c" == '"' ]]; then
                    in_quoted=1
                    ((i++))
                elif [[ "$c" == ',' ]]; then
                    fields+=("$field")
                    field=""
                    ((i++))
                else
                    field+="$c"
                    ((i++))
                fi
            fi
        done
        fields+=("$field")

        # Process fields for output
        unescaped=""
        for (( i=0; i < ${#fields[@]}; i++ )); do
            f="${fields[i]}"
            # Replace "" with " (already handled above, but keep as safeguard)
            f="${f//\"\"/\"}"
            # Replace tab with \t
            f="${f//$'\t'/\\t}"
            # Replace newline with \n
            f="${f//$'\n'/\\n}"
            # Replace backslash with \\ (must be last to avoid double escaping)
            f="${f//\\/\\\\}"
            if (( i > 0 )); then
                unescaped+=$'\t'
            fi
            unescaped+="$f"
        done
        records+=("$unescaped")
    done < "$file"

    for (( i=0; i < ${#records[@]}; i++ )); do
        printf '%s\n' "${records[i]}"
    done
    # Ensure final newline
    printf '\n'
}