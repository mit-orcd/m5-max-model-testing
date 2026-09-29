json_get() {
    local key="$1"
    local json=$(cat)
    local length=${#json}
    local i=0
    local start=0
    local end=0
    local quote=0
    local colon=0
    local is_key=0
    local found_key=0
    local value_start=0
    local value_end=0

    while [ $i -lt $length ]; do
        local char="${json:$i:1}"
        
        if [ "$char" = '"' ] && [ $quote -eq 0 ]; then
            quote=1
        elif [ "$char" = '"' ] && [ $quote -eq 1 ]; then
            quote=0
        fi

        if [ $quote -eq 0 ]; then
            if [ "$char" = ':' ]; then
                colon=1
            elif [ "$char" = '{' ] || [ "$char" = '[' ]; then
                start=$((i + 1))
                quote=1
            elif [ "$char" = '}' ] || [ "$char" = ']' ]; then
                end=$i
                if [ $colon -eq 1 ] && [ $found_key -eq 1 ]; then
                    value_start=$((start + 1))
                    value_end=$((end - 1))
                    printf '%s\n' "${json:$value_start:$((value_end - value_start + 1))}"
                    return
                fi
            fi

            if [ $is_key -eq 1 ] && [ $colon -eq 1 ] && [ $found_key -eq 0 ]; then
                if [ "${json:$start:$((i - start))}" = "$key" ]; then
                    found_key=1
                else
                    is_key=0
                fi
            fi

            if [ "$char" = '"' ] && [ $colon -eq 0 ] && [ $found_key -eq 0 ]; then
                start=$i
                is_key=1
            fi
        fi
        
        i=$((i + 1))
    done
}