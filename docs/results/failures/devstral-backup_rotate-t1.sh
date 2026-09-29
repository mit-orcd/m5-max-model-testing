backup_rotate() {
    local dir="$1"
    local keep="$2"
    local files=($dir/backup-*.tar.gz)

    if (( ${#files[@]} <= keep )); then
        return
    fi

    sort -rV <(printf "%s\n" "${files[@]}") | tail -n +$((keep + 1)) | while read -r file; do
        basename "$file"
    done | sort -V | xargs -I{} rm -f "$dir/{}"
}