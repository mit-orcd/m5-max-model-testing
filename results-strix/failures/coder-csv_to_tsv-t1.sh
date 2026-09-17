csv_to_tsv() {
    local file="$1"
    local IFS=',' read -r -d '' -a fields
    local line
    while IFS= read -r line || [ -n "$line" ]; do
        local result=""
        local field=""
        local in_quotes=false
        local i=0
        local len=${#line}
        local char=""
        local prev_char=""
        
        while [ $i -lt $len ]; do
            char="${line:$i:1}"
            if [ "$char" = '"' ]; then
                if [ "$in_quotes" = false ] && [ "$prev_char" = '"' ]; then
                    field="${field}""'"
                elif [ "$in_quotes" = false ] && [ "$prev_char" != '"' ]; then
                    in_quotes=true
                elif [ "$in_quotes" = true ] && [ "$prev_char" = '"' ]; then
                    field="${field}""'"
                elif [ "$in_quotes" = true ] && [ "$prev_char" != '"' ]; then
                    in_quotes=false
                else
                    field="${field}""'"
                fi
            else
                if [ "$in_quotes" = true ] && [ "$char" = '\n' ]; then
                    field="${field}\\n"
                elif [ "$in_quotes" = true ] && [ "$char" = '\t' ]; then
                    field="${field}\\t"
                else
                    field="${field}${char}"
                fi
            fi
            prev_char="$char"
            i=$((i+1))
        done
        
        if [ -n "$result" ]; then
            result="${result}\t${field}"
        else
            result="${field}"
        fi
        
        echo -e "$result"
    done < "$file"
}