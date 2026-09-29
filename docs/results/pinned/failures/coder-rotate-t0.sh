rotate() {
    local file="$1"
    local backup="${file}.1"
    
    if [[ -f "$backup" ]]; then
        local i=9
        while [[ $i -gt 1 ]]; do
            local prev_backup="${file}.${i}"
            local next_backup="${file}.$((i+1))"
            if [[ -f "$prev_backup" ]]; then
                mv "$prev_backup" "$next_backup"
            fi
            ((i--))
        done
        mv "$file" "${file}.1"
    else
        mv "$file" "${file}.1"
    fi
}