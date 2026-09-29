csv_to_tsv() {
    local file="$1"
    local IFS=',' read -r -d '' -a fields
    local line
    while IFS= read -r line || [[ -n "$line" ]]; do
        local result=""
        local field=""
        local in_quotes=false
        local i=0
        local len=${#line}
        while [[ $i -lt $len ]]; do
            local char="${line:$i:1}"
            if [[ "$char" == '"' ]]; then
                if [[ "$in_quotes" == false ]]; then
                    in_quotes=true
                else
                    if [[ $((i+1)) -lt $len && "${line:$((i+1)):1}" == '"' ]]; then
                        field="${field}""\""
                        i=$((i+1))
                    else
                        in_quotes=false
                    fi
                fi
            elif [[ "$char" == ',' ]] && [[ "$in_quotes" == false ]]; then
                result="${result}${field}\t"
                field=""
            else
                field="${field}${char}"
            fi
            i=$((i+1))
        done
        result="${result}${field}"
        echo -n "$result"
        if [[ -n "$line" ]]; then
            echo
        fi
    done < "$file"
}