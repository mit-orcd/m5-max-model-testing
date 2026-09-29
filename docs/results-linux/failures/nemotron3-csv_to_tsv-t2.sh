csv_to_tsv() {
    local file="$1"
    local line field state quote char escaped next
    state=0  # 0=normal, 1=in quotes
    field=""
    line=""
    escaped=0

    while IFS= read -r -d '' char || [[ -n $char ]]; do
        if (( escaped )); then
            case "$char" in
                n) field+=$'\n' ;;
                t) field+=$'\t' ;;
                *) field+="\\$char" ;;
            esac
            escaped=0
            continue
        fi

        if (( state )); then
            if [[ "$char" == '"' ]]; then
                read -r -n 1 next || break
                if [[ "$next" == '"' ]]; then
                    field+='"'
                else
                    state=0
                    # put back the character for reprocessing
                    printf '%s' "$next" >&3
                    exec 3<&-
                fi
            elif [[ "$char" == $'\n' ]]; then
                field+=$'\n'
            elif [[ "$char" == $'\r' ]]; then
                # ignore CR
                :
            else
                field+="$char"
            fi
        else
            if [[ "$char" == ',' ]]; then
                # end of field
                printf '%s\t' "$field"
                field=""
            elif [[ "$char" == $'\n' ]]; then
                printf '%s\n' "$field"
                field=""
                line=""
            elif [[ "$char" == $'\r' ]]; then
                # ignore CR
                :
            elif [[ "$char" == '"' ]]; then
                state=1
            else
                field+="$char"
            fi
        fi
    done < <(cat "$file"; printf '\0')

    # Handle last field if no trailing newline
    if [[ -n $field ]] || [[ -n $line ]]; then
        printf '%s\n' "$field"
    fi
}