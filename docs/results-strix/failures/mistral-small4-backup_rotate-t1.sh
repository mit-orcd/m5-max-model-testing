backup_rotate() {
    local dir="$1"
    local keep="$2"
    local pattern="backup-*.tar.gz"
    local files=( $(ls -t "$dir/$pattern" 2>/dev/null | tail -n +$((keep + 1)) | sed "s|^$dir/||") )
    for ((i=${#files[@]}-1; i>=0; i--)); do
        echo "${files[i]}"
        rm -f "$dir/${files[i]}"
    done
}