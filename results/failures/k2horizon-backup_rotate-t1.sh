backup_rotate() {
    local dir="$1" keep="$2"
    local backups
    backups=$(ls -1 "$dir"/backup-*.tar.gz 2>/dev/null | sort -r)
    local count
    count=$(printf '%s\n' "$backups" | grep -c .)
    if [ "$count" -le "$keep" ]; then
        return 0
    fi
    printf '%s\n' "$backups" | tail -n "$((count - keep))" | while IFS= read -r f; do
        printf '%s\n' "$(basename "$f")"
        rm "$f"
    done
}