backup_rotate() {
    local dir="$1" keep="$2"
    local files=("$dir"/backup-*.tar.gz)
    local count=${#files[@]}
    if (( count <= keep )); then
        return
    fi
    local sorted=($(printf "%s\n" "${files[@]}" | sort -r))
    local to_delete=("${sorted[@]:keep}")
    for file in "${to_delete[@]}"; do
        basename "$file"
        rm -f "$file"
    done
}