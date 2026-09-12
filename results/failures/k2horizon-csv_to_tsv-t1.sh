csv_to_tsv() {
    local file="$1"
    local line
    local in_quotes=0
    local field=""
    local record=""
    local started=0
    local c
    local i
    local len

    while IFS= read -r -d '' line || [ -n "$line" ]; do
        # Process line; handle CRLF by stripping trailing CR
        line="${line%$'\r'}"
        # Split into characters
        len=${#line}
        for ((i=0; i<len; i++)); do
            c="${line:i:1}"
            if [ "$in_quotes" -eq 1 ]; then
                if [ "$c" = '"' ]; then
                    # Check for escaped quote
                    if [ $((i+1)) -lt $len ] && [ "${line:i+1:1}" = '"' ]; then
                        field+='"'
                        i=$((i+1))
                    else
                        in_quotes=0
                    fi
                elif [ "$c" = $'\n' ]; then
                    field+=$'\\n'
                elif [ "$c" = $'\t' ]; then
                    field+=$'\\t'
                else
                    field+="$c"
                fi
            else
                case "$c" in
                    '"')
                        in_quotes=1
                        ;;
                    ',')
                        record+="$field"
                        record+=$'\t'
                        field=""
                        ;;
                    $'\n')
                        record+="$field"
                        record+=$'\n'
                        printf '%s' "$record"
                        record=""
                        field=""
                        ;;
                    *)
                        field+="$c"
                        ;;
                esac
            fi
        done
    done < <(tr -d '\000' < "$file" | tr '\n' '\001' | tr -d '\n' | tr '\001' '\n')

    # Handle any remaining field/record
    if [ -n "$field" ] || [ -n "$record" ]; then
        record+="$field"
        record+=$'\n'
        printf '%s' "$record"
    else
        printf '\n'
    fi
}