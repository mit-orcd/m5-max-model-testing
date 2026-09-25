rotate() {
    local file="$1"
    local next_level=$(find "$file"* -maxdepth 0 -type f -name "$file.[0-9]*" | sort -t. -k2 -n | tail -n1 | sed -e "s/^$file\.\([0-9]\+\)$/\1/" -e "s/^$file$//")
    if [ -z "$next_level" ]; then
        mv "$file" "$file.1"
    else
        next_level=$((next_level + 1))
        mv "$file" "$file.$next_level"
        mv "$file.1" "$file.2"
    fi
}