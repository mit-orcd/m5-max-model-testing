rotate() {
    local file="$1"
    local backup="${file}.1"
    
    if [ -f "$backup" ]; then
        mv "${file}.1" "${file}.2"
        mv "$file" "${file}.1"
    else
        mv "$file" "$backup"
    }
}