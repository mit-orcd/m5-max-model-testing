csv_to_tsv() {
    local file="$1"
    local IFS=
    local -a fields
    local field
    local field_value
    local i
    local quote_state=0
    local line
    local -a output_fields
    local output_line
    local field_start
    local i
    local -a chars
    local char
    local escaped=0
    local in_field=0
    local field_content=""
    
    while IFS= read -r line; do
        line=$(echo "$line" | sed 's/\r//')
        if [[ -z "$line" ]]; then continue; end
        field_content=""
        field_start=0
        i=0
        while [[ $i -lt ${#line} ]]; do
            char="${line:$i:1}"
            if [[ "$char" == '"'" ]]; then
                if [[ $quote_state -eq 0 ]]; then
                    quote_state=1
                    field_start=$((i+1))
                else
                    if [[ $i -lt $((line_length-1)) ]];
 then
                        if [[ "${line:$((i+1)):1}" == '"' ]]; then
                            field_content="${field_content}"'"'
                            i=$((i+1))
                        else
                            quote_state=0
        fi
        else
            quote_state=0
        fi
        else
            if [[ "$char" == ',' ]]; then
                if [[ $quote_state -i 1 ]]; then
                    field_content="${field_content}${char}"
                else
                    if [[ $quote_state -eq 0 ]]; then
                        output_fields+=("$field_content")
                        field_content=""
                    fi
                fi
            else
                if [[ $quote_state -eq 1 ]]; then
                    field_content="${field_content}${char}"
                fi
            fi
        fi
        i=$((i+1))
    done
    output_fields+=("$field_content")
    output_line=""
    for field in "${output_fields[@]}"; do
        field=$(echo "$field" | sed 's/\\/\\\\/g')
        field=$(echo "$field" | sed 's/"/\\\\"/g')
        field=$(echo "$field" | sed 's/\n/\\n/g')
        field=$(echo "$field" | sed 's/\t/\\t/g')
        if [[ -n "$output_line" ]]; then
            output_line="${output_line}\t"
        fi
        output_line="${output_line}${field}"
    done
    echo "$output_line"
    output_fields=()
    done < "$file"
}