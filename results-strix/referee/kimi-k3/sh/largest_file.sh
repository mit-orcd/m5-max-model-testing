largest_file() {
    find "$1" -type f -exec stat -f '%z %N' {} + | sort -rn | head -1 | cut -d' ' -f2-
}
