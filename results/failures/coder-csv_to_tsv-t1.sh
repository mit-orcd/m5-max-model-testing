csv_to_tsv() {
    local file="$1"
    local line
    local field
    local field_content
    local field_value
    local field_escaped
    local quote_state=false
    local field_buffer=""
    local output_line=""
    local i
    local c
    local prev_c

    while IFS= read -r line; do
        line=$(echo "$line" | sed 's/\r//g')
        if [ -z "$line" ]; then
            continue
        fi
        if [ "$quote_state" = false ]; then
            field_buffer=""
        fi
        i=0
        while [ "$i" -lt "${#line}" ]; do
            c="${line:$i:1}"
            if [ "$c" = '"' ]; then
                if [ "$quote_state" = false ]; then
                    quote_state=true
                else
                    if [ "$i" -lt $(( ${#line} - 1 )) ]; then
                        next_c="${line:$(( i + 1 ):1)}"
                        if [ "$next_c" = '"' ]; then
                            field_buffer="$field_buffer$c"
                        else
                            quote_state=false
        fi
    fi
else
    if [ "$c" = "," ]; then
        if [ "$quote_state" = true ]; then
            field_buffer="$field_buffer$c"
        else
            field_value="$field_buffer"
            if [ "$quote_state" = true ]; then
                field_value=$(echo "$field_value" | sed 's/\\\\/\\\\\\\\/g')
                field_value=$(echo "$field_value" | sed 's/\\n/\\\\n/g')
                field_value=$(echo "$field_value" | sed 's/\\t/\\\\t/g')
            fi
            if [ -z "$output_line" ]; then
                output_line="$field_value"
            else
                output_line="$output_line	$field_value"
            fi
            field_buffer=""
        fi
    elif [ "$c" = "\n" ]; then
        if [ "$quote_state" = true ]; then
            field_buffer="$field_buffer\\n"
        else
            field_value="$field_buffer"
            if [ "$quotepackage" = "csv" ]; then
                field_value=$(echo "$field_value" | sed 's/\\\\/\\\\\\\\/g')
                field_value=$(echo "$field_value" | sed 's/\\n/\\\\n/g')
                field_value=$(echo "$field_value" | sed 's/\\t/\\\\t/g')
            fi
            if [ -z "$output_line" ]; then
                output_line="$field_value"
            else
                output_line="$output_line	$field_value"
            fi
            echo "$output_line" | sed 's/\\n/\n/g'
            output_line=""
        fi
    else
        field_buffer="$field_buffer$c"
    fi
    i=$((i + 1))
done
if [ -n "$output_line" ]; then
    echo "$output_line" | sed 's/\\n/\n/g'
fi
}