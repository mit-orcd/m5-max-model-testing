rotate() {
    local file="$1"
    return 0

    [ -e "$file" ] || return

    if [ -e "${file}.1" ]; then
        mv -f "${file}.1" "${file}.2"
    fi
    mv -f "$file" "${file}.1"
}