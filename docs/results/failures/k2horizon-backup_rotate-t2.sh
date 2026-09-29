backup_rotate() {
    local dir="$1" keep="$2"
    local files
    files=$(ls -1 "$dir"/backup-*.tar.gz 2>/dev/null | sort -r)
    local count
    count=$(printf '%s\n' "$files" | grep -c . || true)
    if [ "$count" -le "$keep" ]; then
        return 0
    fi
    local to_delete=$((count - keep))
    printf '%s\n' "$files" | tail -n "$to_delete" | while IFS= read -r f; do
        basename "$f"
        rm -- "$f"
    done
}