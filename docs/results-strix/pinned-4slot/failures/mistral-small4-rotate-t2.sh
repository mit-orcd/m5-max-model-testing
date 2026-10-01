rotate() {
    local file="$1"
    local next_level=$(find "$file"* -maxdepth 0 -type f -name "$file.[0-9]*" | sort -t. -k2,2nr | head -n 1 | sed -e "s/^$file\.\([0-9]\+\)$/\1/" -e "s/^$file$//")
    if [ -z "$next_level" ]; then
        mv "$file" "$file.1"
    else
        local next_level=$((next_level + 1))
        mv "$file" "$file.$next_level"
        if [ $next_level -gt 1 ]; then
            mv "$file.$((next_level - 1))" "$file.1"
        fi
    fi
}