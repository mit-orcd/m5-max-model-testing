csv_to_tsv() {
    local file="${1:-/dev/stdin}"
    local in_file
    in_file=$(mktemp)
    trap 'rm -f "$in_file"' EXIT
    cp "$file" "$in_file"

    local char prev field content
    local in_quotes=0
    local reading=0
    local first_field=1
    local out_line=""

    while IFS= read -r -n1 char || [[ -n "$char" ]]; do
        case "$char" in
            $'\r')
                continue
                ;;
            '"')
                if [[ $in_quotes -eq 0 ]]; then
                    in_quotes=1
                else
                    # Check next char
                    local next_char
                    IFS= read -r -n1 next_char || next_char=""
                    if [[ "$next_char" == '"' ]]; then
                        field+=$'"'
                        IFS= read -r -n1 next_char || next_char=""
                    else
                        in_quotes=0
                    fi
                fi
                ;;
            $'\n')
                if [[ $in_quotes -eq 1 ]]; then
                    field+=$'\\n'
                else
                    printf '%s\n' "$out_line$field"
                    out_line=""
                    field=""
                    first_field=1
                fi
                ;;
            $'\t')
                if [[ $in_quotes -eq 1 ]]; then
                    field+=$'\\t'
                else
                    if [[ $first_field -eq 1 ]]; then
                        out_line+="$field"
                        first_field=0
                    else
                        out_line+=$'\t'"$field"
                    fi
                    field=""
                fi
                ;;
            ',')
                if [[ $in_quotes -eq 1 ]]; then
                    field+=","
                else
                    if [[ $first_field -eq 1 ]]; then
                        out_line+="$field"
                        first_field=0
                    else
                        out_line+=$'\t'"$field"
                    fi
                    field=""
                fi
                ;;
            *)
                if [[ $in_quotes -eq 1 ]]; then
                    field+="$char"
                else
                    field+="$char"
                fi
                ;;
        esac
    done < "$in_file"

    # Output last field/record
    if [[ $first_field -eq 1 ]]; then
        printf '%s\n' ""
    else
        printf '%s\n' "$out_line$field"
    fi
}