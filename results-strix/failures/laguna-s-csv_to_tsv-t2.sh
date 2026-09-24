csv_to_tsv() {
    local file="$1"
    local line field fields in_quotes content c i n
    in_quotes=0
    fields=()
    field=""
    while IFS= read -r line || [[ -n "$line" ]]; do
        line="${line%$'\r'}"
        i=0
        n=${#line}
        while (( i < n )); do
            c="${line:i:1}"
            if (( in_quotes )); then
                if [[ "$c" == '"' ]]; then
                    if (( i + 1 < n )) && [[ "${line:i+1:1}" == '"' ]]; then
                        field+='"'
                        (( i += 2 ))
                        continue
                    else
                        in_quotes=0
                        (( i += 1 ))
                        continue
                    fi
                else
                    case "$c" in
                        '\t') field+='\t' ;;
                        '\n') field+='\n' ;;
                        *) field+="$c" ;;
                    esac
                    (( i += 1 ))
                    continue
                fi
            else
                case "$c" in
                    '"')
                        in_quotes=1
                        (( i += 1 ))
                        ;;
                    ',')
                        fields+=("$field")
                        field=""
                        (( i += 1 ))
                        ;;
                    *)
                        field+="$c"
                        (( i += 1 ))
                        ;;
                esac
            fi
        done
        if (( in_quotes )); then
            field+=$'\n'
        else
            fields+=("$field")
            printf '%s\n' "$(printf '%s\t' "${fields[@]}")"
            fields=()
            field=""
        fi
    done < "$file"
    if [[ -n "$field" || ${#fields[@]} -gt 0 ]]; then
        fields+=("$field")
        printf '%s\n' "$(printf '%s\t' "${fields[@]}")"
    fi
}