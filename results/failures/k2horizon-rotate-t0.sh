rotate() {
    local file="$1"
    local i=1
    while [ -e "$file.$i" ]; do
        i=$((i + 1))
    done
    if [ "$i" -gt 2 ]; then
        mv "$file.$((i - 1))" "$file.$i"
    fi
    mv "$file" "$file.1"
}