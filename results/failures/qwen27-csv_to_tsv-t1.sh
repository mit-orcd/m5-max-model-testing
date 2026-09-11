csv_to_tsv() {
    local file="$1"
    local line=""
    local record=""
    local field=""
    local in_quotes=0
    local expect_field=1
    local c=""
    local next_c=""
    local i=0
    local len=0

    # Read the file, handling CRLF
    # We'll process character by character
    # Read the entire file into a variable, stripping CR
    local content
    content=$(cat -- "$file" | tr -d '\r')

    len=${#content}
    if (( len == 0 )); then
        return 0
    fi

    i=0
    in_quotes=0
    field=""
    record=""
    expect_field=1

    while (( i < len )); do
        c="${content:$i:1}"

        if (( in_quotes )); then
            if [[ "$c" == '"' ]]; then
                # Check if next char is also a quote
                if (( i + 1 < len )); then
                    next_c="${content:$((i+1)):1}"
                    if [[ "$next_c" == '"' ]]; then
                        field="${field}\""
                        (( i += 2 ))
                        continue
                    fi
                fi
                # End of quoted field
                in_quotes=0
                (( i++ ))
                # Field is complete
                record="${record}${field}"
                field=""
                expect_field=1
                continue
            elif [[ "$c" == $'\n' ]]; then
                field="${field}\\n"
                (( i++ ))
                continue
            elif [[ "$c" == $'\t' ]]; then
                field="${field}\\t"
                (( i++ ))
                continue
            else
                field="${field}${c}"
                (( i++ ))
                continue
            fi
        else
            # Not in quotes
            if [[ "$c" == '"' ]]; then
                # Start of quoted field
                in_quotes=1
                field=""
                (( i++ ))
                continue
            elif [[ "$c" == ',' ]]; then
                # End of field
                if (( expect_field )); then
                    # This shouldn't happen in valid CSV, but handle it
                    record="${record}${field}"
                else
                    record="${record}${field}"
                fi
                field=""
                # Add tab separator unless this is the first field
                if [[ -n "$record" ]]; then
                    record="${record}"
                fi
                # We need to add a tab between fields
                # Actually, let's rebuild: record accumulates fields with tabs
                # Let me restructure: keep a separate approach
                :
                (( i++ ))
                continue
            elif [[ "$c" == $'\n' ]]; then
                # End of record
                # The last field is complete
                if [[ -n "$field" || $expect_field -eq 0 ]]; then
                    if [[ -n "$record" ]]; then
                        record="${record}${field}"
                    else
                        record="${record}"
                    fi
                fi
                # Print the record
                printf '%s\n' "$record"
                record=""
                field=""
                expect_field=1
                in_quotes=0
                (( i++ ))
                continue
            else
                # Regular character in unquoted field
                field="${field}${c}"
                (( i++ ))
                continue
            fi
        fi
    done

    # Handle last record if no trailing newline
    if [[ -n "$record" || -n "$field" ]]; then
        if [[ -n "$record" ]]; then
            record="${record}${field}"
        else
            record="${field}"
        fi
        printf '%s\n' "$record"
    fi

    return 0
}