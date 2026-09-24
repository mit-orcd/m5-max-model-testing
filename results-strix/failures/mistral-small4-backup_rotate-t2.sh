backup_rotate() {
    local dir="$1"
    local keep="$2"
    local files=($(ls -1 "$dir"/backup-*.tar.gz 2>/dev/null | sort))
    local count=${#files[@]}

    if (( count <= keep )); then
        exit 0
    fi

    local to_delete=($(printf "%s\n" "${files[@]}" | head -n $((count - keep)))

    for file in "${to_delete[@]}"; do
        rm "$file"
        basename "$file"
    done
}