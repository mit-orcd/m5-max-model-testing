backup_rotate() {
    local dir="$1"
    local keep="$2"
    # Find matching files, sort lexicographically ascending (oldest first),
    # keep the last <keep> entries (newest), delete the rest
    find "$dir" -maxdepth 1 -type f -name 'backup-*.tar.gz' | sort | head -n -"$keep" | tail -n +1 | while IFS= read -r f; do
        basename "$f"
        rm -f "$f"
    done
}