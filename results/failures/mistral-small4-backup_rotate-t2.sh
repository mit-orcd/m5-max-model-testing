backup_rotate() {
    local dir="$1"
    local keep="$2"
    local count=0
    local files=()

    while IFS= read -r -d '' file; do
        files+=("$file")
    done < <(find "$dir" -maxdepth 1 -type f -name 'backup-*.tar.gz' -print0 | sort -z -r)

    count=${#files[@]}
    if (( count <= keep )); then
        return
    fi

    for (( i=keep; i<count; i++ )); do
        local filename=$(basename "${files[$i]}")
        echo "$filename"
        rm -f "${files[$i]}"
    done
}