csv_to_tsv() {
    local file="$1"
    local state=0  # 0=normal, 1=in quotes
    local field=""
    local line=""
    local c
    local prev

    while IFS= read -r -d '' -n1 c || [[ -n $c ]]; do
        case "$state" in
            0) # normal
                if [[ "$c" == '"' ]]; then
                    state=1
                elif [[ "$c" == $',' ]]; then
                    printf '%s\t' "$line"
                    line=""
                elif [[ "$c" == $'\r' ]]; then
                    # ignore CR
                    continue
                elif [[ "$c" == $'\n' ]]; then
                    printf '%s\n' "$line"
                    line=""
                else
                    line+="$c"
                fi
                ;;
            1) # in quotes
                if [[ "$c" == '"' ]]; then
                    # peek next char
                    IFS= read -r -d '' -n1 prev || break
                    if [[ "$prev" == '"' ]]; then
                        line+='"'
                    else
                        # put back the char for reprocessing
                        printf '%s' "$prev" >&2
                        state=0
                    fi
                elif [[ "$c" == $'\r' ]]; then
                    continue
                elif [[ "$c" == $'\n' ]]; then
                    line+=$'\\n'
                elif [[ "$c" == $'\t' ]]; then
                    line+=$'\\t'
                else
                    line+="$c"
                fi
                ;;
        esac
    done < "$file"

    # output last field and line if any
    if [[ -n "$line" || -n "$field" ]]; then
        printf '%s\n' "$line"
    fi
}