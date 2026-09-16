csv_to_tsv() {
    local file="$1"
    local in_quotes=0
    local current_field=''
    local line=''
    local ch
    local prev_ch=''
    local output_line
    
    while IFS= read -r -n1 ch || [[ -n "$ch" ]]; do
        if [[ "$ch" == $'\r' ]]; then
            continue
        fi
        
        if [[ "$ch" == '"' ]]; then
            if [[ "$in_quotes" -eq 1 ]]; then
                if [[ "$prev_ch" == '"' ]]; then
                    current_field+='"'
                    prev_ch=''
                    continue
                fi
                in_quotes=0
            else
                in_quotes=1
            fi
        elif [[ "$ch" == ',' && "$in_quotes" -eq 0 ]]; then
            if [[ -n "$line" ]]; then
                line+=$'\t'
            fi
            line+="$current_field"
            current_field=''
        elif [[ "$ch" == $'\n' ]]; then
            if [[ "$in_quotes" -eq 1 ]]; then
                current_field+='\n'
            else
                if [[ -n "$line" ]]; then
                    line+=$'\t'
                fi
                line+="$current_field"
                echo "$line"
                line=''
                current_field=''
            fi
        else
            current_field+="$ch"
        fi
        prev_ch="$ch"
    done < "$file"
    
    if [[ -n "$current_field" || -n "$line" ]]; then
        if [[ -n "$line" ]]; then
            line+=$'\t'
        fi
        line+="$current_field"
        echo "$line"
    fi
}