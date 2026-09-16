csv_to_tsv() {
    local file="$1"
    local IFS=',' read -r -d '' -a fields
    local line
    while IFS= read -r line || [ -n "$line" ]; do
        local output=""
        local field=""
        local in_quote=false
        local i=0
        local len=${#line}
        while [ $i -lt $len ]; do
            local char="${line:$i:1}"
            if [ "$char" = '"' ]; then
                if [ "$in_quote" = false ]; then
                    in_quote=true
                else
                    if [ $((i+1)) -lt $len ] && [ "${line:$((i+1)):1}" = '"' ]; then
                        field="${field}\""
                        i=$((i+1))
                    else
                        in_quote=false
                    fi
                fi
            elif [ "$char" = ',' ] && [ "$in_quote" = false ]; then
                if [ -n "$output" ]; then
                    output="${output}\t"
                fi
                output="${output}${field}"
                field=""
            else
                field="${field}${char}"
            fi
            i=$((i+1))
        done
        if [ -n "$output" ]; then
            output="${output}\t"
        fi
        output="${output}${field}"
        echo -n "$output"
        echo ""
    done < "$file"
}