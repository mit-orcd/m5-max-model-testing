csv_to_tsv() {
    local file="$1"
    local IFS=
    local line=""
    local field=""
    local in_quotes=0
    local char
    local output=""
    local first_field=1

    while IFS= read -r -d '' char || [[ -n "$char" ]]; do
        if [[ $in_quotes -eq 1 ]]; then
            if [[ "$char" == '"' ]]; then
                # Check for escaped quote ""
                if [[ -n "$line" && "${line: -1}" == '"' ]]; then
                    field+="${line: -1}"
                    line="${line%?}"
                else
                    # End of quoted field
                    in_quotes=0
                    # Remove surrounding quotes if present at start
                    if [[ "${field:0:1}" == '"' ]]; then
                        field="${field:1}"
                    fi
                fi
            else
                field+="$char"
            fi
        else
            case "$char" in
                '"')
                    in_quotes=1
                    ;;
                ',')
                    if [[ $first_field -eq 1 ]]; then
                        first_field=0
                    else
                        output+=$'\t'
                    fi
                    # Escape tabs and newlines in field before adding
                    field="${field//$'\t'/\\t}"
                    field="${field//$'\n'/\\n}"
                    output+="$field"
                    field=""
                    ;;
                $'\r')
                    # Skip CR
                    ;;
                $'\n')
                    # End of record
                    if [[ $first_field -eq 1 ]]; then
                        first_field=0
                    else
                        output+=$'\t'
                    fi
                    field="${field//$'\t'/\\t}"
                    field="${field//$'\n'/\\n}"
                    output+="$field"
                    echo "$output"
                    output=""
                    first_field=1
                    field=""
                    ;;
                *)
                    field+="$char"
                    ;;
            esac
        fi
    done < "$file"

    # Handle last record if no trailing newline
    if [[ -n "$field" || -n "$output" ]]; then
        if [[ $first_field -eq 1 ]]; then
            first_field=0
        else
            output+=$'\t'
        fi
        field="${field//$'\t'/\\t}"
        field="${field//$'\n'/\\n}"
        output+="$field"
        echo "$output"
    fi
}