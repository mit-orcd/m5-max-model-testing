csv_to_tsv() {
    local file="$1"
    local line field in_quotes=0 i c prev_c escaped=0
    local IFS=$'\n'
    while IFS= read -r line || [[ -n $line ]]; do
        line="${line%$'\r'}"
        field=""
        in_quotes=0
        escaped=0
        for (( i=0; i<${#line}; i++ )); do
            c="${line:i:1}"
            if (( escaped )); then
                case "$c" in
                    n) field+=$'\n' ;;
                    t) field+=$'\t' ;;
                    *) field+="$c" ;;
                esac
                escaped=0
                continue
            fi
            if [[ "$c" == "\\" ]]; then
                escaped=1
                continue
            fi
            if [[ "$c" == '"' ]]; then
                if (( in_quotes )) && (( i+1 < ${#line} )) && [[ "${line:i+1:1}" == '"' ]]; then
                    field+='"'
                    ((i++))
                else
                    in_quotes=$(( !in_quotes ))
                fi
                continue
            fi
            if (( !in_quotes )) && [[ "$c" == "," ]]; then
                printf '%s\t' "$field"
                field=""
                continue
            fi
            field+="$c"
        done
        printf '%s\n' "$field"
    done < "$file"
}