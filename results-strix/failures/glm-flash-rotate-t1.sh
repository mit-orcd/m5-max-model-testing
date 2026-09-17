rotate() {
    local file="$1"
    local count=1
    local next_file="${file}.${count}"

    while [[ -e "$next_file" ]]; do
        ((count++))
        next_file="${file}.${count}"
    done

    # Shift existing versions up one level
    for ((i = count; i >= 2; i--)); do
        mv -f "${file}.${i-1}" "${file}.${i}"
    done

    # Rename the current file to .1
    mv -f "$file" "${file}.1"
}