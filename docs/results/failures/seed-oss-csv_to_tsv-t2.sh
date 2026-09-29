csv_to_tsv() {
    local -a fields quoting=() crlf cr
    local line record field start pos end char esc seq len=${#1}
    local -i i=0 j=0 k=0 quoted=0 escaped=0 in_field=0

    # Read input line by line, handling CRLF/LF
    while IFS= read -r line || [ -n "$line" ]; do
        crlf=$'\r'
        cr=${line##*$'\n'}
        line=${line%$crlf}
        record=()
        quoting=()
        quoted=0
        i=0
        len=${#line}

        # Parse line into fields
        while [ $i -lt $len ]; do
            char=${line:$i:1}
            case "$char" in
                '"')
                    if [ $escaped -eq 1 ]; then
                        seq=$'\\"'
                        escaped=0
                    else
                        if [ $quoted -eq 0 ]; then
                            quoted=1
                            in_field=1
                        else
                            quoted=0
                            in_field=0
                        fi
                    fi
                    ;;
                ',')
                    if [ $quoted -eq 1 ]; then
                        seq+="$char"
                    else
                        field=$seq
                        record+=("$field")
                        seq=''
                        in_field=0
                    fi
                    ;;
                $'\t'|$'\n'|$'\r')
                    if [ $quoted -eq 1 ]; then
                        case "$char" in
                            $'\t') seq+='\\t' ;;
                            $'\n') seq+='\\n' ;;
                            $'\r') seq+=\\r ;;
                        esac
                    else
                        seq+="$char"
                    fi
                    ;;
                '\\')
                    if [ $quoted -eq 1 ]; then
                        if [ $escaped -eq 1 ]; then
                            seq+='\\\\'
                            escaped=0
                        else
                            escaped=1
                        fi
                    else
                        seq+="$char"
                    fi
                    ;;
                *)
                    seq+="$char"
                    ;;
            esac
            i=$((i + 1))
        done

        # Add final field if any
        [ -n "$seq" ] && record+=("$seq")

        # Output record as TSV
        printf "%s\t" "${record[@]}"
        printf '\n'
    done < "$1"

    # Ensure output ends with newline
    printf '\n'
}