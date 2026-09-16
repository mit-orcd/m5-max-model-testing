rotate() {
    local file="$1"
    local backup="${file}.1"
    
    if [[ -f "$backup" ]]; then
        local i=10
        while [[ -f "${file}.${i}" ]]; do
            ((i++))
        done
        while [[ $i -gt 1 ]]; do
            if [[ -f "${file}.${i}" ]]; then
                mv "${file}.${i}" "${file}.$((i+1))"
            fi
            ((i--))
        done
        mv "$file" "${file}.1"
    else
        mv "$file" "${file}.1"
    fi
}