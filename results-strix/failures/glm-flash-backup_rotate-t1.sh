backup_rotate() {
    local dir="$1"
    local keep="$2"
    local target_pattern="backup-*.tar.gz"
    
    # Find files matching the pattern, sort them lexicographically (newest last),
    # and extract just the filenames (not paths).
    local files
    files=$(find "$dir" -maxdepth 1 -type f -name "$target_pattern" -printf "%f\n" | sort -r)

    # Count the files found
    local count
    count=$(echo "$files" | wc -l)

    # If we have fewer or equal files than keep, do nothing
    if (( count <= keep )); then
        return
    fi

    # Calculate how many files to delete (Total - Keep)
    local delete_count=$(( count - keep ))

    # Output the filenames to be deleted (oldest first)
    # We slice the list from the beginning (oldest) up to delete_count
    echo "$files" | head -n "$delete_count"
    
    # Delete the files
    echo "$files" | head -n "$delete_count" | xargs -I {} rm -- "$dir/{}"
}