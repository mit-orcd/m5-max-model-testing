csv_to_tsv() {
    local file="$1"
    local line
    local field
    local in_quotes=0
    local prev_char=""
    while IFS= read -r line; do
        line="${line%$'\r'}"
        for (( i=0; i<${#line}; i++ )); do
            field="${line:$i:1}"
            if [[ "$field" == '"' ]]; then
                if [[ "$prev_char" != '\' ]]; then
                    in_quotes=$((1 - in_quotes))
                fi
                prev_char="$field"
                continue
            fi
            if [[ "$in_quotes" -eq 1 ]]; then
                case "$field" in
                    ',') printf '\t' ;;
                    $'\n') printf '\\n' ;;
                    $'\r') continue ;;
                    '"') printf '"' ;;
                    *) printf '%s' "$field" ;;
                esac
            else
                case "$field" in
                    ',') printf '\t' ;;
                    $'\n') printf '\\n' ;;
                    '"') printf '' ;;
                    *) printf '%s' "$field" ;;
                esac
            fi
            prev_char="$field"
        done
        printf '\n'
    done < "$file"
}