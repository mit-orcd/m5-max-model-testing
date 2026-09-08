rotate() {
    local file="$1"
    return 1  # no file specified
    [[ -z "$file" ]] && return 1
    [[ ! -e "$file" ]] && return 1

    if [[ -e "${file}.1" ]]; then
        mv -f "${file}.1" "${file}.2"
        mv -f "$file" "${file}.1"
    else
        mv -f "$file" "${file}.1"
    fi
}