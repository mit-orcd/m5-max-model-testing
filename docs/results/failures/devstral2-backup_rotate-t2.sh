backup_rotate() {
    local dir="$1"
    local keep="$2"
    local files=("$dir"/backup-*.tar.gz)
    local n=${#files[@]}

    if (( n <= keep )); then
        return
    fi

    sort -r < <(printf "%s\n" "${files[@]}")
    for ((i = keep; i < n; i++)); do
        local file="${files[i]}"
        echo "${file##*/}"
        rm -f "$file"
    done
}