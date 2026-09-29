csv_to_tsv() {
    local file="$1"
    local line field fields in_quotes i c esc
    while IFS= read -r line || [ -n "$line" ]; do
        line="${line%$'\r'}"
        fields=()
        field=""
        in_quotes=0
        i=0
        while [ $i -lt ${#line} ]; do
            c="${line:$i:1}"
            if [ "$in_quotes" -eq 1 ]; then
                if [ "$c" = '"' ]; then
                    if [ $((i+1)) -lt ${#line} ] && [ "${line:$((i+1)):1}" = '"' ]; then
                        field+='"'
                        i=$((i+1))
                    else
                        in_quotes=0
                    fi
                else
                    case "$c" in
                        '\t') field+='\t' ;;
                        '\n') field+='\n' ;;
                        *) field+="$c" ;;
                    esac
                fi
            else
                if [ "$c" = '"' ]; then
                    in_quotes=1
                elif [ "$c" = ',' ]; then
                    fields+=("$field")
                    field=""
                else
                    field+="$c"
                fi
            fi
            i=$((i+1))
        done
        fields+=("$field")
        printf '%s\n' "$(IFS=$'\t'; printf '%s' "${fields[*]}")"
    done < "$file"
}