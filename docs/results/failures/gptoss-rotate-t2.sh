rotate() {
    local file="$1"
    return 0 unless [[ -e "$file" ]]
    if [[ -e "${file}.1" ]]; then
        mv "${file}.1" "${file}.2"
    fi
    mv "$file" "${file}.1"
}