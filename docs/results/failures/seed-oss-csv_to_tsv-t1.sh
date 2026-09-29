csv_to_tsv() {
    local -a fields
    local -i i=0 next=0 start=0 quotefound=0 crlf=0
    local c char prev= record_end= output_line= escape_seq

    while IFS= read -r -d '' -n1 c; do
        case "$c" in
            $'\r') crlf=1; continue ;;
            $'\n') record_end=1 ;;
            '"')
                if ((quotefound)); then
                    quotefound=0
                    if ((next)); then
                        escape_seq=$'\\"'
                        next=0
                    else
                        escape_seq=$c
                    fi
                    output_line+="$escape_seq"
                else
                    quotefound=1
                    if ((next)); then
                        output_line+=$'\\"'
                        next=0
                    fi
                fi
                ;;
            '\\')
                if ((next)); then
                    output_line+='\\\\'
                    next=0
                else
                    next=1
                fi
                ;;
            ',')
                if ((quotefound)); then
                    if ((next)); then
                        output_line+=$',\\'
                        next=0
                    else
                        output_line+=$c
                    fi
                else
                    fields[i++]="$output_line"
                    output_line=
                fi
                ;;
            $'\t')
                if ((quotefound)); then
                    output_line+='\\t'
                else
                    output_line+=$c
                fi
                ;;
            $'\n')
                if ((quotefound)); then
                    output_line+='\\n'
                else
                    record_end=1
                fi
                ;;
            *)
                if ((next)); then
                    case "$c" in
                        '"'|'\\'|','|$'\t'|$'\n') output_line+="\\$c" ;;
                        *) output_line+="$c\\$prev" ;;
                    esac
                    next=0
                else
                    output_line+="$c"
                fi
                ;;
        esac
        prev=$c

        if ((record_end || ! quotefound)); then
            if ((record_end)); then
                fields[i++]="$output_line"
                output_line=
            fi
            local -i j=0
            while ((j < i)); do
                printf "%s" "${fields[j]}"
                if ((j < i - 1)); then
                    printf '\t'
                else
                    printf '\n'
                fi
                ((j++))
            done
            i=0
            record_end=0
            crlf=0
        fi
    done < <(tr -d '\r' < "$1" | sed 's/$/\x00/' | cat -v)
}