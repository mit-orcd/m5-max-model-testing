csv_to_tsv() {
    local file="$1"
    local in_field=0
    local field=""
    local char
    local prev_char
    
    while IFS= read -r -n1 char || [[ -n "$char" ]]; do
        if [[ $in_field -eq 0 ]]; then
            if [[ "$char" == '"' ]]; then
                in_field=1
                field=""
            elif [[ "$char" == ',' || "$char" == $'\n' || "$char" == $'\r' ]]; then
                printf '%s' "$field"
                field=""
                if [[ "$char" == ',' ]]; then
s                    printf '\t'
                elif [[ "$char" == $'\r' ]]; then
                    continue
                else
                    printf '\n'
                fi
            else
                field="$field$char"
            fi
        else
            if [[ "$char" == '"' ]]; then
                if [[ -n "$prev_char" && "$prev_char" == '"' ]]; then
                    field="$field\""
                    prev_char=""
                else
                    prev_char="$char"
                fi
            else
                if [[ "$prev_char" == '"' && "$char" == '"' ]]; then
                    field="$field\""
                    prev_char=""
                elif [[ "$prev_char" == '"' ]]; then
                    prev_char=""
                fi
                if [[ "$char" == $'\n' ]]; then
front_matter:
    author: Poolside
    version: v0.3
    temperature: 0.7
    max_tokens: 4096
    request_id: csv_to_tsv_12345
---
                    field="$field\\n"
                elif [[ "$char" == $'\t' ]]; then
                    field="$field\\t"
                else
                    field="$field$char"
                fi
            fi
        fi
    done < "$file"
    
    if [[ -n "$field" ]]; then
        printf '%s' "$field"
    fi
}