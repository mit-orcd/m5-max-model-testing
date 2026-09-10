backup_rotate() {
    local dir="$1" keep="$2"
    local files
    files=$(find "$dir" -maxdepth 1 -type f -name 'backup-*.tar.gz' -printf '%f\n' | sort -r)
    local total
    total=$(printf '%s\n' "$files" | grep -c . || true)
    local delete_count=$((total - keep))
    if (( delete_count > 0 )); then
        printf '%s\n' "$files" | tail -n "$delete_count" | while IFS= read -r f; do
            rm -f -- "$dir/$f"
            printf '%s\n' "$f"
        done
    fi
}