csv_to_tsv() {
    local file=${1:-/dev/stdin}
    local line buf inquote=0

    # -----------------------------------------------------------------
    # Helper: parse a single CSV record (may contain embedded newlines)
    # -----------------------------------------------------------------
    parse_record() {
        local rec=$1
        local i=0 len=${#rec}
        local field='' quoted=0 out=''

        while (( i < len )); do
            local c=${rec:i:1}
            if (( quoted )); then
                if [[ $c == '"' ]]; then
                    # look ahead for escaped quote
                    local nxt=${rec:i+1:1}
                    if [[ $nxt == '"' ]]; then
                        field+=\"
                        ((i++))
                    else
                        quoted=0
                    fi
                else
                    case $c in
                        $'\t') field+='\\t' ;;
                        $'\n') field+='\\n' ;;
                        *)     field+=$c ;;
                    esac
                fi
            else
                if [[ $c == ',' ]]; then
                    out+="${field}"
                    out+=$'\t'
                    field=''
                elif [[ $c == '"' ]]; then
                    quoted=1
                else
                    case $c in
                        $'\t') field+='\\t' ;;
                        $'\n') field+='\\n' ;;
                        *)     field+=$c ;;
                    esac
                fi
            fi
            ((i++))
        done
        out+="${field}"
        printf '%s\n' "$out"
    }

    # -----------------------------------------------------------------
    # Read the input, joining lines that belong to the same CSV record
    # -----------------------------------------------------------------
    while IFS= read -r line || [[ -n $line ]]; do
        line=${line%$'\r'}                     # strip possible CR
        if (( inquote )); then
            buf=$buf$'\n'$line
        else
            buf=$line
        fi

        # Remove escaped double‑quotes ("" → empty) and count remaining quotes
        local tmp=$buf
        while [[ $tmp == *\"\"* ]]; do
            tmp=${tmp//\"\"/}
        done
        local qcnt=$(awk -v s="$tmp" 'BEGIN{gsub(/[^"]/,"",s); print length(s)}')

        if (( qcnt % 2 == 0 )); then
            parse_record "$buf"
            inquote=0
            buf=''
        else
            inquote=1
        fi
    done < "$file"

    # Final record without trailing newline
    if [[ -n $buf ]]; then
        parse_record "$buf"
    fi
}