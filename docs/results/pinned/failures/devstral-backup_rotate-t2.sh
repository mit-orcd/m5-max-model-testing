backup_rotate() {
    local dir="$1"
    local keep="$2"
    local files=()
    local deleted=()

    # Get all backup files, sort lexicographically (newest first)
    while IFS= read -r -d '' file; do
        files+=("$file")
    done < <(find "$dir" -maxdepth 1 -name 'backup-*.tar.gz' -print0 | sort -z -r)

    # Check if we have more files than we want to keep
    if (( ${#files[@]} > keep )); then
        # Delete oldest files (from position $keep onwards)
        for ((i=keep; i<${#files[@]}; i++)); do
            local filename=$(basename "${files[i]}")
            rm -f "${files[i]}"
            deleted+=("$filename")
        done
    fi

    # Print deleted files, oldest first
    for filename in "${deleted[@]}"; do
        echo "$filename"
    done
}