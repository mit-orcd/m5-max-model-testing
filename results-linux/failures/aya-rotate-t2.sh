rotate() {
    local file="$1"
    local tmpfile="${file}.tmp"
    local newfile="${file}.1"
    local oldfile="${file}"

    if [[ -f "$file" && ! -f "$newfile" ]]; then
        mv "$file" "$newfile"
    fi

    if [[ -f "$file" ]]; then
        cp "$file" "$tmpfile"
        mv "$tmpfile" "$oldfile"
        mv "$newfile" "$file"
    fi
}