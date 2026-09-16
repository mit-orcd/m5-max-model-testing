csv_to_tsv() {
    local file="$1"
    local line field fields_in_line in_quotes char c i n state=0
    local -a fields=()
    while IFS= read -r -d '' line || [[ -n "$line" ]]; do
        line="${line%$'\r'}"
        state=0
        field=""
        in_quotes=0
        for ((i=0; i<${#line}; i++)); do
            c="${line:$i:1}"
            if (( in_quotes )); then
                if [[ "$c" == '"' ]]; then
                    if (( i+1 < ${#line} && "${line:$((i+1)):1}" == '"' )); then
                        field+='"'
                        ((i++))
                    else
                        in_quotes=0
                    fi
                else
                    field+="$c"
                fi
            else
                case "$c" in
                    '"') in_quotes=1 ;;
                    ',') fields+=("$field"); field="" ;;
                    *) field+="$c" ;;
                esac
            fi
        done
        fields+=("$field")
        printf '%s\n' "$(printf '%s\t' "${fields[@]}")" | sed 's/\t$//'
        fields=()
    done < <(tr -d '\r' < "$file" | sed 's/\x0/\\x0/g; s/\t/\\t/g' | awk -v RS='\n' '{gsub(/\\x0/, "\x0"); print}')
}