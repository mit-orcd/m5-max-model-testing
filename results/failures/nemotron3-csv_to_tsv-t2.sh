csv_to_tsv() {
    local file="$1"
    local state=0  # 0: unquoted, 1: quoted
    local field=""
    local line=""
    local c
    local prev=''

    while IFS= read -r -d '' -n1 c || [[ -n $c ]]; do
        if [[ $c == $'\r' ]]; then
            continue
        fi

        case $state in
            0)  # unquoted
                case "$c" in
                    ',')
                        line+="${field}$'\t'"
                        field=""
                        ;;
                    '"')
                        state=1
                        field=""
                        ;;
                    $'\n')
                        line+="${field}"
                        printf '%s\n' "$line"
                        line=""
                        field=""
                        ;;
                    *)
                        field+="$c"
                        ;;
                esac
                ;;
            1)  # quoted
                if [[ "$c" == '"' ]]; then
                    if [[ "$prev" == '"' ]]; then
                        field+='"'
                        prev=''
                    else
                        prev='"'
                    fi
                else
                    if [[ -n $prev ]]; then
                        prev=''
                    fi
                    case "$c" in
                        $'\n')
                            field+=$'\\n'
                            ;;
                        $'\t')
                            field+=$'\\t'
                            ;;
                        *)
                            field+="$c"
                            ;;
                    esac
                fi
                ;;
        esac
    done < "$file"

    # Output last field and line if any
    if [[ -n $field || -n $line ]]; then
        line+="${field}"
        printf '%s\n' "$line"
    elif [[ -n $line ]]; then
        printf '%s\n' "$line"
    fi
}