rotate() {
    local file="$1"
    local backup="${file}.1"
    
    if [[ -f "$backup" ]]; then
        # Shift existing backups
        local i=9
        while [[ $i -gt 1 ]]; do
            local prev_backup="${file}.${i}"
            local next_backup="${file}.$((i+1))"
            if [[ -f "$prev_backup" ]]; then
                mv "$prev_backup" "$next_backup"
            fi
            ((i--))
        done
        # Move the main file to .1
        mv "$file" "$backup"
    else
        # Just rename the file to .1
        mv "$file" "$backup"
    fi
}