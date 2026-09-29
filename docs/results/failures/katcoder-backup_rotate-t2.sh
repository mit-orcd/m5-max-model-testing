backup_rotate() {
    local dir="$1"
    local keep="$2"
    local -a files
    mapfile -t files < <(find "$dir" -maxdepth 1 -type f -name 'backup-*.tar.gz' -printf '%f\n' | sort)
    local count=${#files[@]}
    if (( count <= keep )); then
        return
    fi
    local -a to_delete=("${files[@]:0:count-keep}")
    for f in "${to_delete[@]}"; do
        rm -- "$dir/$f"
        echo "$f"
    done
}