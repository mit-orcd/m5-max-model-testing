rotate() {
    local file="$1"
    local backup="${file}.1"
    
    if [[ -f "$backup" ]]; then
        # Rotate all existing backups
        local i=9
        while [[ $i -gt 1 ]]; do
            local old_backup="${file}.${i}"
            local new_backup="${file}.$((i+1))"
            if [[ -f "$old_backup" ]]; then
                mv "$old_backup" "$new_backup"
            fi
            ((i--))
        done
        # Move the current file to .1
        mv "$file" "$backup"
    else
        # If .1 doesn't exist, just rename the file to .1
        mv "$file" "$backup"
    fi
}