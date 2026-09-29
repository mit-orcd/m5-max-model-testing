csv_to_tsv() {
    local file="$1"
    local line
    local in_quotes=0
    local field
    local output_line=""
    local escape=0

    while IFS= read -r line || [[ -n "$line" ]]; do
        local i=0
        local len=${#line}
        field=""
        while (( i < len )); do
            case "$line:$i" in
                *\"*)
                    if (( !in_quotes )); then
                        in_quotes=1
                        ((i++))
                    else
                        if (( escape )); then
                            field+="\""
                            escape=0
                            ((i++))
                        else
                            if (( i + 1 < len && line:$i:$i == "\" && line:$((i+1)):$((i+1)) == "\" )); then
                                field+="\""
                                ((i += 2))
                            else
                                in_quotes=0
                                ((i++))
                            fi
                        fi
                    fi
                    ;;
                *\\*)
                    if (( in_quotes )); then
                        escape=1
                        ((i++))
                    else
                        field+="\\"
                        ((i++))
                    fi
                    ;;
                *,$)
                    if (( in_quotes )); then
                        field+=","
                    else
                        output_line+="$field"
                        field=""
                    fi
                    ((i++))
                    ;;
                *$'\r')
                    if (( in_quotes )); then
                        field+="\\n"
                    fi
                    ((i++))
                    ;;
                *$'\t')
                    if (( in_quotes )); then
                        field+="\\t"
                    fi
                    ((i++))
                    ;;
                *)
                    field+="${line:$i:1}"
                    ((i++))
                    ;;
            esac
        done

        if (( in_quotes )); then
            field+="${line:$i}"
        fi

        output_line+="$field"
        echo -e "${output_line//$'\r'}"
        output_line=""
        in_quotes=0
    done < "$file"
    echo
}