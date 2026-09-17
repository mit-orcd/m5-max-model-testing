rotate() {
    local file="$1"
    local base="${file%.1}"
    local next_num=2

    # Find the highest existing rotation number
    for f in "${file}".[0-9]*; do
        [[ -f "$f" ]] || continue
        local num="${f##*.}"
        [[ "$num" =~ ^[0-9]+$ ]] || continue
        if (( num >= next_num )); then
            next_num=$((num + 1))
        fi
    done

    # Rotate existing numbered files in reverse order
    while (( next_num > 2 )); do
        local prev=$((next_num - 1))
        if [[ -f "${file}.${prev}" ]]; then
            mv "${file}.${prev}" "${file}.${next_num}"
        fi
        ((next_num--))
    done

    # Rename the current file to .1
    if [[ -f "$file" ]]; then
        mv "$file" "${file}.1"
    fi
}