total_size() {
    local dir=$1
    local sum=0
    # Use process substitution to avoid a subshell for the while loop
    while IFS= read -r -d '' file; do
        # %z gives size in bytes on macOS/BSD stat
        size=$(stat -f %z "$file")
        sum=$((sum + size))
    done < <(find "$dir" -type f -print0 2>/dev/null)
    printf '%s\n' "$sum"
}