backup_rotate() {
    local dir="$1"
    local keep="$2"
    # Collect matching files, sort lexicographically (oldest first), keep the first (total - keep) for deletion
    local files_to_delete
    files_to_delete=$(cd "$dir" && printf '%s\n' backup-*.tar.gz 2>/dev/null | sort | head -n -"$keep" 2>/dev/null)
    # If there are files to delete, print each (basename) on its own line
    if [ -n "$files_to_delete" ]; then
        while IFS= read -r file; do
            basename "$file"
        done <<< "$files_to_delete"
    fi
}