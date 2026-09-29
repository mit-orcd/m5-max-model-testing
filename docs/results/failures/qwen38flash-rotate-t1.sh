rotate() {
    local file="$1"
    local i=1
    while [ -e "${file}.${i}" ]; do
        mv "${file}.${i}" "${file}.$((i + 1))"
        i=$((i + 1))
    done
    mv "$file" "${file}.1"
}