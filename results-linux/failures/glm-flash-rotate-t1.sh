rotate() {
    local file="$1"
    local base="${file%.*}"
    local ext="${file##*.}"

    if [[ "$file" == "$base" ]]; then
        base="$file"
        ext=""
    fi

    for i in $(seq 10 -1 2); do
        if [[ -e "${base}.${i}.${ext}" ]]; then
            mv -f "${base}.${i}.${ext}" "${base}.$((i+1)).${ext}"
        fi
    done

    if [[ -e "${base}.1.${ext}" ]]; then
        mv -f "${base}.1.${ext}" "${base}.2.${ext}"
    fi

    mv -f "$file" "${base}.1.${ext}"
}