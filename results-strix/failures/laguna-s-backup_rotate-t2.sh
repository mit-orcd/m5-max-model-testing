backup_rotate() {
    local dir="$1"
    local keep="$2"
    local files=()
    while IFS= read -r file; do
        files+=("$file")
    done < <(ls -1 "$dir"/backup-*.tar.gz 2>/dev/null | sort -r)
    local count=${#files[@]}
    if (( count <= keep )); then
        return 0
    fi
    for (( i=keep; i<count; i++ )); do
        local filename="${files[$i]##*/}"
        rm -f "${files[$i]}"
        echo "$filename"
    done
}