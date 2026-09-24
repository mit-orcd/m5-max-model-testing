csv_to_tsv() {
    local src="${1:-/dev/stdin}"
    exec 3<"$src"

    local char next pushback inquote field
    local -a fields

    while :; do
        # get next character, using any pushed‑back character first
        if [[ -n $pushback ]]; then
            char=$pushback
            pushback=
        else
            IFS= read -r -n1 -u 3 char || break
        fi

        # strip CR (handle CRLF)
        [[ $char == $'\r' ]] && continue

        if (( inquote )); then
            if [[ $char == '"' ]]; then
                # look ahead to see if this is an escaped quote
                IFS= read -r -n1 -u 3 next || next=
                if [[ $next == '"' ]]; then
                    field+='"'                     # escaped quote
                    continue
                else
                    inquote=0                      # end of quoted field
                    pushback=$next                 # re‑process following char
                    continue
                fi
            else
                # inside quoted field: escape TAB and NL
                case $char in
                    $'\t') field+='\\t' ;;
                    $'\n') field+='\\n' ;;
                    *)     field+="$char" ;;
                esac
                continue
            fi
        fi

        # not inside a quoted field
        case $char in
            '"')  inquote=1 ;;
            ',')  fields+=("$field"); field= ;;
            $'\n')
                fields+=("$field")
                # output the accumulated fields as TSV
                printf '%s' "${fields[0]}"
                for ((i=1; i<${#fields[@]}; i++)); do
                    printf '\t%s' "${fields[i]}"
                done
                printf '\n'
                fields=()
                field=
                ;;
            $'\t') field+='\\t' ;;
            *)     field+="$char" ;;
        esac
    done

    # handle final record without a trailing newline
    if [[ -n $field || ${#fields[@]} -gt 0 ]]; then
        fields+=("$field")
        printf '%s' "${fields[0]}"
        for ((i=1; i<${#fields[@]}; i++)); do
            printf '\t%s' "${fields[i]}"
        done
        printf '\n'
    fi

    exec 3<&-
}