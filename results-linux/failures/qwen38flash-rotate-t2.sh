rotate() {
    local file="$1"
    local max=1
    local i

    # Find the highest existing index
    while [[ -e "${file}.${((max + 1))}" ]]; do
        max=$((max + 1))
    done

    # Shift existing files: .n -> .(n+1), starting from highest down to 1
    for ((i = max; i >= 1; i--)); do
        if [[ -e "${file}.${i}" ]]; then
            mv "${file}.${i}" "${file}.$((i + 1))"
        fi
    done

    # Rename current file to .1
    if [[ -e "$file" ]]; then
        mv "$file" "${file}.1"
    fi
}