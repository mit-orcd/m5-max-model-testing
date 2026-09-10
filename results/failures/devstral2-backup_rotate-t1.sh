backup_rotate() {
    local dir="$1" keep="$2"
    local files=($(ls -1 "$dir"/backup-*.tar.gz 2>/dev/null | sort -r))
    if (( ${#files[@]} > keep )); then
        for ((i=keep; i<${#files[@]}; i++)); do
            local file="${files[$i]}"
            file="${file##*/}"
            echo "$file"
            rm -f "$dir/$file"
        done
    fi
}