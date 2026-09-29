csv_col() {
    local name="$1"
    local header=""
    local col_idx=-1
    local line_num=0
    local field=""
    local in_quotes=0
    local escaped=0
    local fields=()
    local current_field=""
    local i char

    # Read header line
    IFS= read -r header
    if [ $? -ne 0 ] || [ -z "$header" ]; then
        return
    fi

    # Parse header to find column index
    fields=()
    current_field=""
    in_quotes=0
    for (( i=0; i<${#header}; i++ )); do
        char="${header:i:i}"
        if [ $in_quotes -eq 1 ]; then
            if [ "$char" = '"' ]; then
                if (( i+1 < ${#header} )) && [ "${header:i+1:i+1}" = '"' ]; then
                    current_field+='"'
                    ((i++))
                else
                    in_quotes=0
                fi
            else
                current_field+="$char"
            fi
        else
            if [ "$char" = '"' ]; then
                in_quotes=1
            elif [ "$char" = ',' ]; then
                fields+=("$current_field")
                current_field=""
            else
                current_field+="$char"
            fi
        fi
    done
    fields+=("$current_field")

    # Find the index of the target column
    for (( i=0; i<${#fields[@]}; i++ )); do
        if [ "${fields[i]}" = "$name" ]; then
            col_idx=$i
            break
        fi
    done

    if [ $col_idx -eq -1 ]; then
        return
    fi

    # Process each data row
    while IFS= read -r line || [ -n "$line" ]; do
        if [ -z "$line" ]; then
            continue
        fi
        fields=()
        current_field=""
        in_quotes=0
        for (( i=0; i<${#line}; i++ )); do
            char="${line:i:i}"
            if [ $in_quotes -eq 1 ]; then
                if [ "$char" = '"' ]; then
                    if (( i+1 < ${#line} )) && [ "${line:i+1:i+1}" = '"' ]; then
                        current_field+='"'
                        ((i++))
                    else
                        in_quotes=0
                    fi
                else
                    current_field+="$char"
                fi
            else
                if [ "$char" = '"' ]; then
                    in_quotes=1
                elif [ "$char" = ',' ]; then
                    fields+=("$current_field")
                    current_field=""
                else
                    current_field+="$char"
                fi
            fi
        done
        fields+=("$current_field")

        if [ $col_idx -lt ${#fields[@]} ]; then
            printf '%s\n' "${fields[col_idx]}"
        fi
    done
}