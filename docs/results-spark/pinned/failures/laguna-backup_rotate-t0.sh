backup_rotate() {
    local dir="$1"
    local keep="$2"
    local count
    count=$(find "$dir" -maxdepth 1 -name 'backup-*.tar.gz' | wc -l)
    if (( count <= keep )); then
        return
    fi
    find "$dir" -maxdepth 1 -name 'backup-*.tar.gz' | sort | head -n -keep | while IFS= read -r file; do
        basename "$file"
        rm "$file"
    done
}