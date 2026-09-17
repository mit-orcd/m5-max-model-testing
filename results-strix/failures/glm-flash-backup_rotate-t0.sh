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
    local delete_count=$(( ${#files[@]} - keep ))

    # Print the oldest files (last in the array) and delete them
    for (( i = delete_count; i < ${#files[@]}; i++ )); do
        local filename="${files[$i]##*/}"
        echo "$filename"
        rm -f "${files[$i]}"
    done
}