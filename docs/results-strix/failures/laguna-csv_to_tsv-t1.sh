csv_to_tsv() {
    local file="$1"
    local -a fields
    local field='' line='' in_quotes=0
    local -i i len c prev

    while IFS= read -r line || [[ -n $line ]]; do
        line="${line%$'\r'}"
        i=0; len=${#line}
        fields=()
        field=''
        in_quotes=0

        while i < len; do
            c="${line:i:1}"
            case "$c" in
                '"')
                    if (( in_quotes )); then
                        if (( i + 1 < len )); then
                            next="${line:((i+1)):1}"
                            if [[ "$next" == '"' ]]; then
                                field+='"'
                                ((i++))
                            else
                                in_quotes=0
                            fi
                        else
                            in_quotes=0
                        fi
                    else
                        in_quotes=1
                    fi
                    ;;
                $'\t')
                    if (( in_quotes )); then
                        field+=$'\t'
                    else
                        fields+=("$field")
                        field=''
                    fi
                    ;;
                $'\n')
                    if (( in_quotes )); then
                        field+='\n'
                    else
                        fields+=("$field")
                        printf '%s\n' "$(IFS=$'\t'; echo "${fields[*]}")"
                        fields=()
                        field=''
                    fi
                    ;;
                ',')
                    if (( in_quotes )); then
                        field+=','
                    else
                        fields+=("$field")
                        field=''
                    fi
                    ;;
                *)
                    field+="$c"
                    ;;
            esac
            ((i++))
        done

        if (( in_quotes )); then
            field+=$'\n'
        fi

        fields+=("$field")
        printf '%s\n' "$(
            for f in "${fields[@]}"; do
                printf '%s\t' "${f//$'\n'/\\n}" "${f//$'\t'/\\t}"
            done | sed 's/\t$//'
        )"
    done < "$file"
}