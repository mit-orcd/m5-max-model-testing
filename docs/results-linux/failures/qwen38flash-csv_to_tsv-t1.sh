csv_to_tsv() {
    local file="$1"
    local field=""
    local line=""
    local in_quotes=0
    local prev_char=""
    local char
    local i=0
    local len
    local data
    local output=""

    # Read entire file into data
    data=$(cat "$file")
    len=${#data}

    # Process character by character
    while [ "$i" -lt "$len" ]; do
        char="${data:$i:1}"
        i=$((i + 1))

        if [ "$in_quotes" -eq 1 ]; then
            if [ "$char" = '"' ]; then
                # Check if next char is also a quote (escaped quote)
                if [ "$i" -lt "$len" ] && [ "${data:$i:1}" = '"' ]; then
                    field="${field}\""
                    i=$((i + 1))
                else
                    # End of quoted field
                    in_quotes=0
                fi
            elif [ "$char" = $'\n' ]; then
                # Line break inside quoted field -> \n
                field="${field}\\n"
            elif [ "$char" = $'\r' ]; then
                # CR inside quoted field - check if followed by LF
                if [ "$i" -lt "$len" ] && [ "${data:$i:1}" = $'\n' ]; then
                    field="${field}\\n"
                    i=$((i + 1))
                else
                    # Lone CR, treat as newline
                    field="${field}\\n"
                fi
            elif [ "$char" = $'\t' ]; then
                # Tab inside quoted field -> \t
                field="${field}\\t"
            else
                field="${field}${char}"
            fi
        else
            if [ "$char" = '"' ]; then
                # Start of quoted field
                in_quotes=1
            elif [ "$char" = ',' ]; then
                # Field separator
                line="${line}${field}"
                field=""
                line="${line}\t"
            elif [ "$char" = $'\n' ]; then
                # End of record (LF)
                line="${line}${field}"
                field=""
                # Remove trailing tab if line is empty due to trailing comma
                # But we keep empty fields, so just output
                if [ -n "$line" ] || [ -n "$field" ]; then
                    # Check if line ends with tab from previous separator
                    # Actually we want to output the line as-is
                    output="${output}${line}\n"
                    line=""
                fi
            elif [ "$char" = $'\r' ]; then
                # CR - check if followed by LF
                if [ "$i" -lt "$len" ] && [ "${data:$i:1}" = $'\n' ]; then
                    # CRLF
                    line="${line}${field}"
                    field=""
                    output="${output}${line}\n"
                    line=""
                    i=$((i + 1))
                else
                    # Lone CR - treat as newline? RFC says CRLF or LF, but handle gracefully
                    line="${line}${field}"
                    field=""
                    output="${output}${line}\n"
                    line=""
                fi
            elif [ "$char" = $'\t' ]; then
                # Tab outside quotes - should not happen in valid CSV, but handle
                field="${field}\\t"
            else
                field="${field}${char}"
            fi
        fi
    done

    # Handle last field/line
    if [ -n "$field" ] || [ -n "$line" ]; then
        line="${line}${field}"
        output="${output}${line}\n"
    fi

    # Output result, ensuring it ends with newline
    printf '%s' "$output"
}