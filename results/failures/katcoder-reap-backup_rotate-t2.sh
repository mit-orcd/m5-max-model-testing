backup_rotate() {
    local dir="$1" keep="$2"
    local -a files
    mapfile -t files "$dir/backup-"*.tar.gz
    local n=${#files[@]}
    if (( n <= keep )); then
        return
    fi
    local -a to_delete=("${files[@]:0:n-keep}")
    for f in "${to_delete[@]}"; do
        local base
        base=$(basename -- "$f")
        echo "$base"
        rm -- "$f"
    done
}