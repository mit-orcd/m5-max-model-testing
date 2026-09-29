largest_file() {
    local dir="$1"
    find "$dir" -type f -exec stat --format='%s %n' {} + 2>/dev/null | sort -rn | head -n 1 | cut -d' ' -f2-
}