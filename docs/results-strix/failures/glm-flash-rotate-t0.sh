rotate() {
    local file="$1"
    local count=1
    local max_count=9

    while [ "$count" -le "$max_count" ]; do
        local next_file="${file}.${count}"
        local current_file="${file}.$((count - 1))"

        if [ -e "$next_file" ]; then
            mv "$next_file" "$current_file"
        else
            mv "$file" "$next_file"
            return
        fi
        count=$((count + 1))
    done

    # If we reach here, the file was renamed to .9, so we remove .9
    rm -f "${file}.9"
}