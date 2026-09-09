rotate() {
    local file="$1"
    if [ -f "${file}.1" ]; then
        rotate "${file}.1"
    fi
    mv -f "$file" "${file}.1"
}