total_size() {
    local dir="$1"
    local sum=0
    # Find all regular files, handle filenames with spaces and newlines using null termination
    while IFS= read -r -d '' file; do
        # Check if it is a regular file (f is a symlink, l is a link to a dir)
        if [ -f "$file" ]; then
            # Use stat -f%z for macOS BSD
            # Handle potential stat errors gracefully by ignoring non-readable files
            stat -f%z "$file" 2>/dev/null | {
                read -r size
                sum=$((sum + size))
            }
        fi
    done < <(find "$dir" -type f -print0)
    printf '%s' "$sum"
}