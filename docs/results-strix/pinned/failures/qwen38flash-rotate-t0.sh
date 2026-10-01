rotate() {
    local file="$1"
    if [ ! -f "${file}.1" ]; then
        mv "$file" "${file}.1"
    else
        local i=1
        while [ -f "${file}.${i}" ]; do
            mv "${file}.${i}" "${file}.$((i + 1))"
            i=$((i + 1))
        done
        mv "$file" "${file}.1"
    fi
}