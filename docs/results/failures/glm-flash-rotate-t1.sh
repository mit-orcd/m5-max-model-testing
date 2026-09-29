rotate() {
    local file="$1"
    if [[ -f "${file}.1" ]]; then
        mv "${file}.9" "${file}.9.bak" 2>/dev/null || true
        for ((i=9; i>=2; i--)); do
            if [[ -f "${file}.${i}" ]]; then
                mv "${file}.${i}" "${file}.$((i+1))"
            fi
        done
        mv "${file}.1" "${file}.9"
    fi
    mv "${file}" "${file}.1"
}