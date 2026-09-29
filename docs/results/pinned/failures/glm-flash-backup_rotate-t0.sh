backup_rotate() {
    local dir="$1"
    local keep="$2"
    local pattern="backup-*.tar.gz"
    local files=()

    # Find files matching the pattern and sort them lexicographically (newest first)
    while IFS= read -r -d '' file; do
        files+=("$file")
    done < <(find "$dir" -maxdepth 1 -type f -name "$pattern" -print0 | sort -z)

    # If we have fewer or equal files than keep, do nothing
    if (( ${#files[@]} <= keep )); then
        return
    fi

    # Calculate how many files to delete
    local to_delete=$(( ${#files[@]} - keep ))

    # Print the oldest files (last in the array) one per line
    for (( i=0; i<to_delete; i++ )); do
        local file="${files[$(( ${#files[@]} - 1 - i ))]}"
        # Extract just the filename from the full path
        local filename=$(basename "$file")
        echo "$filename"
    done
}