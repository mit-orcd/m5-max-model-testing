csv_to_tsv() {
    local file="$1"
    local IFS=
    local -a fields
    local in_field=0
    local in_quotes=0
    local field=""
    local c
    local prev_c=""
    local char_code

    # Read file character by character
    while IFS= read -r -n 1 c || [[ -n "$c" ]]; do
        # Handle CR (CRLF or standalone)
        if [[ "$c" == $'\r' ]]; then
            continue
        fi

        # Handle end of record (LF)
        if [[ "$c" == $'\n' ]]; then
            if [[ $in_quotes -eq 1 ]]; then
                # Unescaped newline inside quotes -> \n
                field="${field}\\n"
                in_field=1
                continue
            fi

            # End of record
            printf '%s' "$field"
            for ((i=1; i<${#fields[@]}; i++)); do
                printf '\t%s' "${fields[i]}"
            done
            printf '\n'
            fields=()
            field=""
            in_field=0
            in_quotes=0
            prev_c=""
            continue
        fi

        # Handle EOF if no final newline
        if [[ -z "$c" && -z "$prev_c" && $in_field -eq 0 ]]; then
            break
        fi

        # Handle double quotes
        if [[ "$c" == '"' ]]; then
            if [[ $in_quotes -eq 1 ]]; then
                # Check for escaped quote ""
                if [[ "$prev_c" == '"' ]]; then
                    # Remove the previous quote from field
                    field="${field%?}"
                    field="${field}\""
                    in_quotes=0
                    in_field=1
                else
                    # Start or end of quoted section
                    if [[ $in_field -eq 0 ]]; then
                        in_quotes=1
                    else
                        # Unexpected quote in middle of unquoted field (treat as literal? RFC says start)
                        # Assuming valid RFC, this marks end if we were in quotes, but we handled above
                        # If we are here, we are not in quotes. So this is start of quoted field.
                        in_quotes=1
                    fi
                fi
                prev_c="$c"
                continue
            else
                # Start of quoted field
                if [[ $in_field -eq 0 ]]; then
                    in_quotes=1
                else
                    # Quote in middle of unquoted field - strictly RFC 4180 implies this is invalid or literal
                    # But usually implies end of field if at start. Let's assume valid input starts with quote if in_quotes.
                    # If we are not in quotes and get a quote, it starts a quoted field.
                    in_quotes=1
                fi
                prev_c="$c"
                continue
            fi
        fi

        # Handle comma
        if [[ "$c" == ',' ]]; then
            if [[ $in_quotes -eq 1 ]]; then
                # Literal comma inside quotes
                field="${field}${c}"
            else
                # End of field
                fields+=("$field")
                field=""
                in_field=0
            fi
            prev_c="$c"
            continue
        fi

        # Handle tabs
        if [[ "$c" == $'\t' ]]; then
            if [[ $in_quotes -eq 1 ]]; then
                # Tab inside quotes -> \t
                field="${field}\\t"
            else
                # Literal tab outside quotes (invalid per RFC usually, but treat as literal)
                field="${field}${c}"
            fi
            in_field=1
            prev_c="$c"
            continue
        fi

        # Regular character
        field="${field}${c}"
        in_field=1
        prev_c="$c"
    done < "$file"

    # Handle last record if no trailing newline
    if [[ -n "$field" || ${#fields[@]} -gt 0 ]]; then
        printf '%s' "$field"
        for ((i=1; i<${#fields[@]}; i++)); do
            printf '\t%s' "${fields[i]}"
        done
        printf '\n'
    fi
}