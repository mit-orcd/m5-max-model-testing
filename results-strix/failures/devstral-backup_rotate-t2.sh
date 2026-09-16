backup_rotate() {
    local dir="$1"
    local keep="$2"
    local files=($(ls -t "$dir"/backup-*.tar.gz | grep -E 'backup-\d{8}-\d{6}\.tar\.gz' | sort -r))
    if (( ${#files[@]} > keep )); then
        local to_delete=("${files[@]:keep}")
        for file in "${to_delete[@]}"; do
            echo "${file##*/}"
            rm -f "$file"
        done
    fi
}