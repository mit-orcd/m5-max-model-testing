csv_to_tsv() {
    local file="$1"
    local line
    local in_quotes=0
    local output_line=""
    local prev_char=""

    while IFS= read -r line || [[ -n "$line" ]]; do
        local i=0
        local len=${#line}
        while (( i < len )); do
            char="${line:i:1}"
            case "$char" in
                '"')
                    if (( in_quotes )); then
                        # Check for escaped quote
                        if (( i < len - 1 )) && [[ "${line:i+1:1}" == '"' ]]; then
                            output_line+='"'
                            (( i += 2 ))
                        else
                            in_quotes=0
                            (( i += 1 ))
                        fi
                    else
                        in_quotes=1
                        (( i += 1 ))
                    fi
                    ;;
                $'\r')
                    # Remove CR
                    (( i += 1 ))
                    ;;
                $'\n')
                    # Handle newline in quoted field
                    if (( in_quotes )); then
                        output_line+="\\n"
                        (( i += 1 ))
                    else
                        # End of line outside quotes
                        break
                    fi
                    ;;
                $'\t')
                    # Handle tab in quoted field
                    if (( in_quotes )); then
                        output_line+="\\t"
                        (( i += 1 ))
                    else
                        # Tab outside quotes is field separator
                        output_line+=$'\t'
                        (( i += 1 ))
                    fi
                    ;;
                ',')
                    if (( in_quotes )); then
                        output_line+=","
                        (( i += 1 ))
                    else
                        # Comma outside quotes is field separator
                        output_line+=$'\t'
                        (( i += 1 ))
                    fi
                    ;;
                *)
                    output_line+="$char"
                    (( i += 1 ))
                    ;;
            esac
        done

        # Remove trailing tab if field ends with one
        if [[ "$output_line" =~ $'\t'$ ]]; then
            output_line="${output_line%$'\t'}"
        fi

        echo -n "$output_line"
        output_line=""
    done < "$file"

    # Ensure output ends with newline
    echo
}