rotate() {
    local file=$1
    local newfile=${file}.1
    if [[ -f "$newfile" ]]; then
        mv "$file" "${file}.2"
        mv "$newfile" "$file"
    else
        mv "$file" "$newfile"
    fi
}