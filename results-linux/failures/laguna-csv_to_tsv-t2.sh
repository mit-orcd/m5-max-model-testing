csv_to_tsv() {
    local file="$1"
    local in_quotes=0
    local field=''
    local line=()
    local -a buffer=()
    
    while IFS= read -r line || [[ -n $line ]]; do
        line="${line%$'\r'}"
        local pos=0
        local len=${#line}
        
        while (( pos <= len )); do
            if (( pos < len )); then
                char="${line:pos:1}"
            else
                char=''
            fi
            
            if (( in_quotes )); then
                if [[ "$char" == '"' ]]; then
                    next="${line:pos+1:1}"
                    if [[ "$next" == '"' ]]; then
                        field+='"'
                        (( pos += 2 ))
                        continue
                    else
                        in_quotes=0
                        (( pos += 1 ))
                        continue
                    fi
                else
                    field+="$char"
                    (( pos += 1 ))
                fi
            else
                if [[ "$char" == '"' ]]; then
                    in_quotes=1
                    (( pos += 1 ))
                elif [[ "$char" == ',' ]]; then
                    line+= "$(printf '%s' "$field" | sed 's/\t/\\t/g; s/\n/\\n/g')"
                    field=''
                    (( pos += 1 ))
                elif [[ "$char" == $'\t' ]]; then
                    line+= "$(printf '%s' "$field" | sed 's/\t/\\t/g; s/\n/\\n/g')"
                    field=''
                    (( pos += 1 ))
                elif [[ "$char" == $'\n' ]]; then
                    if [[ -n "$field" ]] || (( ${#line[@]} > 0 )); then
                        line+= "$(printf '%s' "$field" | sed 's/\t/\\t/g; s/\n/\\n/g')"
                        field=''
                    fi
                    if (( ${#line[@]} > 0 )); then
                        printf '%s\n' "${line[*]}"
                    fi
                    line=()
                    (( pos += 1 ))
                elif [[ -z "$char" ]]; then
                    if [[ -n "$field" ]] || (( ${#line[@]} > 0 )); then
                        line+= "$(printf '%s' "$field" | sed 's/\t/\\t/g; s/\n/\\n/g')"
                        field=''
                    fi
                    if (( ${#line[@]} > 0 )); then
                        printf '%s\n' "${line[*]}"
                    fi
                    line=()
                    break
                else
                    field+="$char"
                    (( pos += 1 ))
                fi
            fi
        done
        
        if (( in_quotes )); then
            field+='\n'
        fi
    done < "$file"
    
    if [[ -n "$field" ]] || (( ${#line[@]} > 0 )); then
        line+= "$(printf '%s' "$field" | sed 's/\t/\\t/g; s/\n/\\n/g')"
        printf '%s\n' "${line[*]}"
    fi
}