backup_rotate() {
    local dir="$1" keep="$2"
    local files=("$dir"/backup-*.tar.gz)
    [[ ! -e "${files[0]}" ]] && return
    printf '%s\n' "${files[@]##*/}" | sort -r | tail -n +$((keep + 1)) | while read -r f; do
        rm -f "$dir/$f"
        printf '%s\n' "$f"
    done
}