csv_to_tsv() {
    local file="${1:-/dev/stdin}"
    local -i state=0 field="" in_field=0 escaped=0
    local line c out_field out_line output=""
    
    while IFS= read -r line || [[ -n "$line" ]]; do
        # Strip trailing CR
        line="${line%$'\r'}"
        
        local len=${#line}
        local i=0
        
        while [[ $i -lt $len ]]; do
            c="${line:$i:1}"
            i=$((i + 1))
            
            if [[ $escaped -eq 1 ]]; then
                if [[ "$c" == '"' ]]; then
                    field+="\""
                else
                    field+="$c"
                fi
                escaped=0
                continue
            fi
            
            if [[ $in_field -eq 0 ]]; then
                if [[ "$c" == '"' ]]; then
                    in_field=1
                elif [[ "$c" == $'\t' ]]; then
                    output+=$'\t'"$field"
                    field=""
                elif [[ "$c" == $'\n' ]]; then
                    output+=$'\n'"$field"
                    field=""
                else
                    field+="$c"
                fi
            else
                if [[ "$c" == '"' ]]; then
                    if [[ $i -lt $len ]]; then
                        local nc="${line:$i:1}"
                        if [[ "$nc" == '"' ]]; then
                            field+="\""
                            i=$((i + 1))
                        else
                            in_field=0
                        fi
                    else
                        in_field=0
                    fi
                elif [[ "$c" == $'\t' ]]; then
                    field+="\\t"
                elif [[ "$c" == $'\n' ]]; then
                    field+="\\n"
                else
                    field+="$c"
                fi
            fi
        done
        
        case $state in
            0)
                if [[ $in_field -eq 1 ]]; then
                    state=1
                else
                    state=2
                fi
                ;;
            1)
                if [[ $in_field -eq 0 ]]; then
                    state=2
                fi
                ;;
            2)
                output+=$'\n'"$field"
                field=""
                state=0
                ;;
        esac
    done < "$file"
    
    if [[ $state -eq 2 ]]; then
        printf '%s\n' "$output$field"
    elif [[ $state -eq 1 ]]; then
        # unterminated quoted field - emit as-is
        printf '%s\n' "$output$field"
    elif [[ -n "$output" ]]; then
        printf '%s\n' "$output"
    fi
}