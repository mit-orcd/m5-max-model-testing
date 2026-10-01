rotate() {
    local file="$1"
    local max=2
    for ((i = max - 1; i >= 1; i--)); do
        if [ -e "$file.$i" ]; then
            if [ "$i" -eq 1 ]; then
                mv -f "$file" "$file.1"
            else
                mv -f "$file.$i" "$file.$((i + 1))"
            fi
        fi
    done
}