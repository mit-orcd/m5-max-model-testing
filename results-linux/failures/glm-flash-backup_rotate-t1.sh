backup_rotate() {
    local dir="$1"
    local keep="$2"
    local -a files

    # Find all backup files, sort them lexicographically (newest first),
    # and store them in an array.
    while IFS= read -r -d '' file; do
        files+=("$file")
    done < <(find "$dir" -maxdepth 1 -type f -name "backup-*.tar.gz" -print0 | sort -z)

    # If fewer or equal files exist than 'keep', do nothing.
    if (( ${#files[@]} <= keep )); then
        return
    fi

    # Determine how many files to delete (excess files).
    local delete_count=$(( ${#files[@]} - keep ))

    # Iterate over the array from the oldest end to the newest end.
    # The array is sorted newest first, so the last elements are the oldest.
    for (( i=${#files[@]} - 1; i >= delete_count; i-- )); do
        # Print the filename (basename) of the file to be deleted.
        printf '%s\n' "${files[i]##*/}"
    done
}