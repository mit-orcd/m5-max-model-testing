csv_to_tsv() {
    local file="$1"
    local line=""
    local field=""
    local in_quotes=0
    local record=""
    local first_field=1
    local c
    local i=0
    local len
    local -a fields=()

    # Read the entire file, handling CRLF and LF
    # We'll process character by character
    local content
    content=$(cat "$file" 2>/dev/null) || return 1

    # Add a sentinel newline to ensure the last record is processed
    content="${content}"$'\n'

    len=${#content}
    i=0
    in_quotes=0
    field=""
    fields=()
    first_field=1

    while (( i < len )); do
        c="${content:$i:1}"

        if (( in_quotes )); then
            if [[ "$c" == '"' ]]; then
                # Check if next char is also a quote (escaped quote)
                if (( i + 1 < len )) && [[ "${content:$((i+1)):1}" == '"' ]]; then
                    field="${field}""
                    (( i++ ))
                else
                    # End of quoted field
                    in_quotes=0
                    (( i++ ))
                    # Field is complete
                    if (( first_field )); then
                        record="${field}"
                        first_field=0
                    else
                        record="${record}"$'\t'"${field}"
                    fi
                    field=""
                    continue
                fi
            else
                # Inside quotes: handle special characters
                if [[ "$c" == $'\n' ]]; then
                    field="${field}\\n"
                elif [[ "$c" == $'\t' ]]; then
                    field="${field}\\t"
                else
                    field="${field}${c}"
                fi
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
                # Field separator
                if (( first_field )); then
                    record="${field}"
                    first_field=0
                else
                    record="${record}"$'\t'"${field}"
                fi
                field=""
            elif [[ "$c" == $'\r' ]]; then
                # CR: skip it (part of CRLF or standalone)
                # Don't add to field
                :
            elif [[ "$c" == $'\n' ]]; then
                # End of record
                if (( first_field )); then
                    record="${field}"
                else
                    record="${record}"$'\t'"${field}"
                fi
                printf '%s\n' "$record"
                record=""
                field=""
                first_field=1
            else
                field="${field}${c}"
            fi
        fi
        (( i++ ))
    done
}