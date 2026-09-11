csv_to_tsv() {
    local file="$1"
    local line buffer field quoted value
    local IFS=
    
    while IFS= read -r line || [[ -n "$line" ]]; do
        # Normalize line endings
        line="${line%$'\r'}"
        
        buffer=""
        field=""
        quoted=0
        
        for ((i=0; i<${#line}; i++)); do
            char="${line:$i:1}"
            
            if [[ $quoted -eq 1 ]]; then
                if [[ "$char" == '"' ]]; then
                    if [[ "${line:$((i+1)):1}" == '"' ]]; then
                        field+='"'
                        ((i++))
                    else
                        quoted=0
                    fi
                else
                    field+="$char"
                fi
            else
                case "$char" in
                    '"')
                        quoted=1
                        ;;
                    ',')
                        # Escape special chars in field
                        value="${field//\\/\\\\}"
                        value="${value//$'\n'/\\n}"
                        value="${value//$'\t'/\\t}"
                        buffer="${buffer}${value}"$'\t'
                        field=""
                        ;;
                    *)
                        field+="$char"
                        ;;
                esac
            fi
        done
        
        # Handle last field
        value="${field//\\/\\\\}"
        value="${value//$'\n'/\\n}"
        value="${value//$'\t'/\\t}"
        buffer="${buffer}${value}"
        
        printf '%s\n' "$buffer"
    done < "$file"
}