backup_rotate() {
    local dir="$1" keep="$2"
    local files
    files=$(find "$dir" -maxdepth 1 -type f -name 'backup-*.tar.gz' -printf '%f\n' | sort -r)
    local count
    count=$(printf '%s\n' "$files" | grep -c .)
    if [ "$count" -le "$keep" ]; then
        return 0
    fi
    local to_delete=$((count - keep))
    printf '%s\n' "$files" | tail -n "$to_delete" | sort
    printf '%s\n' "$files" | tail -n "$to_delete" | while IFS= read -r f; do
        rm -- "$dir/$f"
    done
}