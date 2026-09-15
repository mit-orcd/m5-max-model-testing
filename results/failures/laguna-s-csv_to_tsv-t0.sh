csv_to_tsv() {
    local file="$1"
    local line field fields_in_line in_quotes char i n
    local -a fields=()
    local current_field=""
    local record=""
    local output=""
    local c

    # Read entire file into a variable, handling CRLF
    local content
    content=$(cat "$file" | tr -d '\r')
    n=${#content}
    i=0
    in_quotes=0
    current_field=""
    fields=()
    output=""

    while [ $i -lt $n ]; do
        c="${content:$i:1}"
        if [ "$in_quotes" -eq 1 ]; then
            if [ "$c" = '"' ]; then
                # Check for escaped quote
                if [ $((i+1)) -lt $n ] && [ "${content:$((i+1)):1}" = '"' ]; then
                    current_field+='"'
                    i=$((i+2))
                    continue
                else
                    in_quotes=0
                    i=$((i+1))
                    continue
                fi
            else
                current_field+="$c"
                i=$((i+1))
                continue
            fi
        else
            if [ "$c" = '"' ]; then
                in_quotes=1
                i=$((i+1))
                continue
            elif [ "$c" = ',' ]; then
                fields+=("$current_field")
                current_field=""
                i=$((i+1))
                continue
            elif [ "$c" = $'\n' ]; then
                fields+=("$current_field")
                current_field=""
                # Process the record
                record=""
                for field in "${fields[@]}"; do
                    # Escape tabs and newlines
                    field="${field//\\/\\\\}"
                    field="${field//$'\t'/\\t}"
                    field="${field//$'\n'/\\n}"
                    if [ -z "$record" ]; then
                        record="$field"
                    else
                        record+=$'\t'"$field"
                    fi
                done
                output+="$record"$'\n'
                fields=()
                i=$((i+1))
                continue
            else
                current_field+="$c"
                i=$((i+1))
                continue
            fi
        fi
    done

    # Handle last field/record if no trailing newline
    if [ -n "$current_field" ] || [ ${#fields[@]} -gt 0 ]; then
        fields+=("$current_field")
        current_field=""
        record=""
        for field in "${fields[@]}"; do
            field="${field//\\/\\\\}"
            field="${field//$'\t'/\\t}"
            field="${field//$'\n'/\\n}"
            if [ -z "$record" ]; then
                record="$field"
            else
                record+=$'\t'"$field"
            fi
        done
        output+="$record"$'\n'
    fi

    printf '%s' "$output"
}