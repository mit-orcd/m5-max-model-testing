rotate() {
    local file="$1"
    local backup="${file}.1"
    
    if [[ -f "$backup" ]]; then
        local i=9
        while [[ $i -gt 1 ]]; do
            local src="${file}.${i-1}"
            local dst="${file}.${i}"
            if [[ -f "$src" ]]; then
                mv "$src" "$dst"
            fi
            ((i--))
        done
        mv "$file" "${file}.1"
    else
        mv "$file" "${file}.1"
    fi
}